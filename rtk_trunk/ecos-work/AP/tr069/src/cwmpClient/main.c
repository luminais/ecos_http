#include <signal.h>

#define	SIGCHLD	20	/* to parent on child stop or exit */
#include "main.h"
static main_args client_args;
#include "prmt_igd.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif
#ifdef CONFIG_APP_TR104
#include "cwmp_main_tr104.h"
#endif
#ifdef CONFIG_USER_CWMP_WITH_TR181
#include "tr181_device.h"
#endif
/* Ecos reinit */
#ifdef HAVE_SYSTEM_REINIT
#include "sys_init.h"
#endif

void cwmp_show_help( void )
{
	//fprintf( stderr, "cwmpClient:\n" );
	//fprintf( stderr, "	-SendGetRPC:	send GetRPCMethods to ACS\n" );
	//fprintf( stderr, "	-SSLAuth:	ACS need certificate the CPE\n" );
	//fprintf( stderr, "	-SkipMReboot:	do not send 'M Reboot' event code\n" );
	//fprintf( stderr, "	-Delay: 	delay some seconds to start\n" );
	//fprintf( stderr, "	-NoDebugMsg: 	do not show debug message\n" );
	//fprintf( stderr, "	-h or -help: 	show help\n" );
	//fprintf( stderr, "\n" );
	//fprintf( stderr, "	if no arguments, read the setting from mib\n" );
	//fprintf( stderr, "\n" );
}

/*refer to climenu.c*/
#define CWMP_RUNFILE	"/var/run/cwmp.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CWMP_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	// 通过PID号生成临时的文件名
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void clr_pid()
{
		unlink(CWMP_RUNFILE);
#ifdef CONFIG_MIDDLEWARE
		unlink(CWMP_MIDPROC_RUNFILE);
#endif
}

#ifdef CONFIG_MIDDLEWARE
extern void handle_alarm(cyg_addrword_t sig);
extern void updateMidprocTimer();

void cwmp_handle_alarm(int sig)
{
	handle_alarm(0);
	updateMidprocTimer();
}

void handle_x_ct_event(int sig)
{
	unsigned char vChar;
	unsigned int vUint=0;
	
	//printf("\n%s\n",__FUNCTION__);
	mib_get(CWMP_TR069_ENABLE,(void*)&vChar);
	if(vChar == 1 || vChar == 2){
		if( mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)!=0 ){
			mib_get(MIB_MIDWARE_INFORM_EVENT,(void *)&vChar);
			switch(vChar){
			case CTEVENT_ACCOUNTCHANGE:
				vUint = vUint|(EC_X_CT_COM_ACCOUNT);
				break;
#ifdef _PRMT_X_CT_COM_USERINFO_
			case CTEVENT_BIND:
				vUint = vUint|(EC_X_CT_COM_BIND);
				break;
#endif
#ifdef E8B_NEW_DIAGNOSE
			case CTEVENT_SEND_INFORM:
				vUint = vUint|(EC_X_CT_COM_SEND_INFORM);
				break;
#endif
			default:
				break;
			}
			mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
}
#else
void handle_x_ct_account(int sig)
{
	unsigned int vUint=0;

	warn("%s():%d ", __FUNCTION__, __LINE__);
	if (mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)) {
		vUint = vUint | EC_X_CT_COM_ACCOUNT;
		mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
	}
}

#ifdef _PRMT_X_CT_COM_USERINFO_
void handle_x_ct_bind(int sig)
{
	unsigned int vUint;

	warn("%s():%d ", __FUNCTION__, __LINE__);
	if (mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)) {
		vUint = vUint | EC_X_CT_COM_BIND;
		mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
	}	
}
#endif
#endif //#ifdef CONFIG_MIDDLEWARE


/*star:20091229 START send signal to cwmp process to let it know that wan connection ip changed*/
void sigusr1_handler()
{
	notify_set_wan_changed();
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	setTr069QosFlag(0);
#endif
/*star:20100305 END*/
}
/*star:20091229 END*/

void clear_child(int i)
{
	int status;
	pid_t chidpid = 0;

	//chidpid=wait( &status );
#ifdef _PRMT_TR143_
#ifdef CONFIG_USER_FTP_FTP_FTP
	//if(chidpid!=-1)
		checkPidforFTPDiag( chidpid );
#endif //CONFIG_USER_FTP_FTP_FTP
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
//	if(chidpid!=-1)
		checkPidforTraceRouteDiag( chidpid );
#endif //_SUPPORT_TRACEROUTE_PROFILE_
	return;
}

void handle_term()
{
	clr_pid();
#ifdef CONFIG_APP_TR104	
	tr104_main_exit();
#endif
	exit(0);
}

int cwmpClient_main(main_args *client_args)
{
	//diag_printf("enter cwmpClient_main\n");

	int i;
	int argc = client_args->argc;
	char *argv[24];
	for(i=0; (i<argc) && (i<24); i++){
		argv[i] = client_args->argv[i];
	}

	// 获取进程号和cwmp文件名字
	log_pid();

	// 初始化apmib
	apmib_init();
	
	// 指定信号处理函数
#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef CONFIG_MIDDLEWARE
	signal(SIGUSR1, handle_x_ct_event);		//xl_yue: SIGUSR2 is used by midware 
#else
	signal( SIGUSR1,handle_x_ct_account);
#ifdef _PRMT_X_CT_COM_USERINFO_
	signal(SIGUSR2, handle_x_ct_bind);
#endif
#endif
#else
	signal(SIGUSR1, sigusr1_handler);
#endif

	signal(SIGCHLD, clear_child);	//set this signal process function according to CWMP_TR069_ENABLE below if MIDDLEWARE is defined
	signal( SIGTERM,handle_term);
	
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	setTr069QosFlag(0);
#endif
/*star:20100305 END*/

	//if( argc >= 2 )
	//diag_printf("Deal with args, argc=%d\n",argc);
#if 1
	int cwmp_flag_in_mib=0;
	mib_get(MIB_CWMP_FLAG, (void *)&cwmp_flag_in_mib);
	//diag_printf("cwmp_flag_in_mib=%d\n", cwmp_flag_in_mib);
	if(!(cwmp_flag_in_mib&CWMP_FLAG_AUTORUN)){ //if this equals to 0, means in CWMP_FLAG is not right
		cwmp_flag_in_mib = cwmp_flag_in_mib | CWMP_FLAG_AUTORUN;
		mib_set(MIB_CWMP_FLAG, &cwmp_flag_in_mib);
	}
	mib_get(MIB_CWMP_FLAG, (void *)&cwmp_flag_in_mib);
	//diag_printf("cwmp_flag_in_mib=%d\n", cwmp_flag_in_mib);
#endif
	if( argc >= 1 )
	{
		int i;
		//for(i=1;i<argc;i++)
		//by cairui
		for(i=0;i<argc;i++)
		{
			if( strcmp( argv[i], "-SendGetRPC" )==0 )
			{
				cwmpinit_SendGetRPC(1);
				//fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
			}else if( strcmp( argv[i], "-SSLAuth" )==0 )
			{
				cwmpinit_SSLAuth(1);
				//fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
			}else if( strcmp( argv[i], "-SkipMReboot" )==0 )
			{
				cwmpinit_SkipMReboot(1);
				//fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
			}else if( strcmp( argv[i], "-Delay" )==0 )
			{
				cwmpinit_DelayStart(30);
				//fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
			}else if( strcmp( argv[i], "-NoDebugMsg" )==0 )
			{
				cwmpinit_NoDebugMsg(1);
				//fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );
			}else if( strcmp( argv[i], "-h" )==0 || strcmp( argv[i], "-help" )==0 )
			{
				cwmp_show_help();
				//exit(0);
			}else
			{
				//fprintf( stderr, "<%s>Error argument: %s\n", __FILE__,argv[i] );
			}
		}
	}else{
		unsigned int cwmp_flag=0;
		//read the flag, CWMP_FLAG, from mib
		if ( mib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag)!=0 )
		{
			//diag_printf("\ncwmp_flag=0x%x\n",cwmp_flag);
			// CWMP_FLAG_DEBUG_MSG = 0x1;
			if( (cwmp_flag&CWMP_FLAG_DEBUG_MSG)==0 )
			{
				//fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );
				cwmpinit_NoDebugMsg(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_CERT_AUTH )
			{
				//fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
				cwmpinit_SSLAuth(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_SENDGETRPC )
			{
				//fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
				cwmpinit_SendGetRPC(1);
			}
			
			if( cwmp_flag&CWMP_FLAG_SKIPMREBOOT )
			{
				//fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
				cwmpinit_SkipMReboot(1);
			}
				
			if( cwmp_flag&CWMP_FLAG_DELAY )
			{
				//fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
				cwmpinit_DelayStart(30);
			}

			if( cwmp_flag&CWMP_FLAG_SELFREBOOT)
			{
				//fprintf( stderr, "<%s>Set SelfReboot!\n", __FILE__ );
				cwmpinit_SelfReboot(1);
			}
				
		}

		// read CWMP_FLAG2 from MIB
		if ( mib_get( MIB_CWMP_FLAG2, (void *)&cwmp_flag)!=0 )
		{
			//diag_printf("Read CWMP_FLAG2, cwmp_flag=0x%x\n",cwmp_flag);
#if defined(CONFIG_BOA_WEB_E8B_CH) || defined(_TR069_CONREQ_AUTH_SELECT_)
			if( cwmp_flag&CWMP_FLAG2_DIS_CONREQ_AUTH)
			{
				//fprintf( stderr, "<%s>Set DisConReqAuth!\n", __FILE__ );
				cwmpinit_DisConReqAuth(1);
			}
#endif

			if( cwmp_flag&CWMP_FLAG2_DEFAULT_WANIP_IN_INFORM)
				cwmpinit_OnlyDefaultWanIPinInform(1);
			else
				cwmpinit_OnlyDefaultWanIPinInform(0);
		}		
	}

#ifdef TELEFONICA_DEFAULT_CFG
//#ifdef WLAN_SUPPORT
	cwmpinit_BringLanMacAddrInInform(1);
//#endif //WLAN_SUPPORT
	cwmpinit_SslSetAllowSelfSignedCert(0);
#endif //TELEFONICA_DEFAULT_CFG

/*star:20100105 START when there is already a session, if return 503 after receive connection request*/
	cwmpinit_Return503(0);
/*star:20100105 END*/
#ifdef CONFIG_APP_TR104
	//cwmp_solarOpen();
	tr104_main_init();
#endif
//startRip();
	//printf("from cwmpClient_main enter cwmp_main!\n");
//#ifdef CONFIG_MIDDLEWARE
//	cwmp_main(mw_tROOT);
//#else
	// enter cwmp_main
#ifdef CONFIG_USER_CWMP_WITH_TR181
	diag_printf("Root is Device:2.\n");
	cwmpinit_UseTR181(1);
	cwmp_main( tDevROOT );
#else
	diag_printf("Root is IGD.\n");
	cwmpinit_UseTR181(0);
	cwmp_main( tROOT );
#endif
//#endif
//	cyg_thread_resume(tr069_thread);
	
	int ret;
	cyg_handle_t handle1;
	cyg_thread_info info1;
	ret = get_thread_info_by_name("cwmpClient", &info1);
	handle1 = info1.handle;
	if(&handle1 != NULL){
		cyg_thread_kill(handle1);     
		ret = cyg_thread_delete(handle1);
		//diag_printf("delete cwmpClient, ret=%d\n", ret);
	}
	//diag_printf("get out of cwmpClient_main\n");
	return 0;
}

/* tr069_start==0 : init, ==1 : has been started
 */
int tr069_start(int argc, char *argv[])
{
	int i,j;
	//diag_printf("argc=%d\n", argc);
	client_args.argc = argc;
	for(i=0; i<argc; i++){
		client_args.argv[i] = argv[i];
		//for(j=0; client_args.argv[i][j]!='\0';j++)
			//diag_printf("%c", client_args.argv[i][j]);
		//printf("\n");
	}

	//diag_printf("client_args set OK\n");
	while(1){
		//diag_printf("enter tr069_start, tr069_startd=%d\n", tr069_startd);
		if(tr069_startd==0)
		{
			//init tr069 thread with name "cwmpClient"
			cyg_thread_create(TR069_THREAD_PRIORITY,
					cwmpClient_main,
					&client_args,  
					"cwmpClient",
					tr069_stack,
					sizeof(tr069_stack),
					&tr069_thread,
					&tr069_thread_object);
			cyg_thread_resume(tr069_thread);

			//diag_printf("create tr069, argc=%d, \n", argc);

			//cyg_thread_suspend(tr069_thread);
			tr069_startd=1;

			return(0);
		} 
		else{
			//diag_printf("tr069 is running, argc=%d, \n", argc);
			cyg_thread_resume(tr069_thread);
			cwmpClient_main(&client_args);

			//cyg_thread_suspend(tr069_thread);
			//diag_printf("get out of tr069_start\n");
			return(-1);
		}
	}//end while1
}

#ifdef HAVE_SYSTEM_REINIT
int tr069_exit()
{
    diag_printf("Exit TR069...\n");
    int ret=0;
    if(tr069_startd){
		cwmp_webClient_exit();
    	//diag_printf("cyg_thread_kill(tr069_thread)\n");
    	cyg_thread_kill(tr069_thread);
    	//diag_printf("cyg_thread_delete(tr069_thread)\n");
		cyg_thread_delete(tr069_thread);
		tr069_startd = 0;
		return ret;
    }

    return 1;
}
#endif
