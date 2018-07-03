#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "prmt_igd.h"
#include "prmt_ippingdiag.h"
#include "prmt_wancondevice.h"
#include "prmt_wandsldiagnostics.h"
#include "prmt_wanatmf5loopback.h"
#include "prmt_deviceinfo.h"
#ifdef _PRMT_TR143_
#include "prmt_tr143.h"
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
#include "prmt_traceroute.h"
#endif //_SUPPORT_TRACEROUTE_PROFILE_
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "prmt_captiveportal.h"
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
#include "prmt_landevice.h"
#endif //SUPPORT_DHCP_RESERVED_IPADDR
#if defined(CONFIG_DEV_xDSL)
#include <config/autoconf.h>
#endif
#ifdef CONFIG_APP_TR104
#include "cwmp_main_tr104.h"
#endif
#if defined(ECOS_MEM_CHAIN_API)
extern unsigned int fileSize;
#endif

#define CWMP_HTTP_REALM		"realtek.com.tw"
#define CONFIG_DIR		"/var/cwmp_config"
/*notification*/
//#define	NOTIFY_FILENAME		CONFIG_DIR"/CWMPNotify.txt"
#define	NOTIFY_FILENAME		CONFIG_DIR"/DefaultCwmpNotify.txt"
#ifdef CONFIG_MIDDLEWARE
#define	MW_NOTIFY_FILENAME		CONFIG_DIR"/MWNotify.txt"
#endif
/*certificates*/
#define CA_FNAME		CONFIG_DIR"/cacert.pem"
#define CERT_FNAME		CONFIG_DIR"/client.pem"
#define DEF_CA_FN		"/etc/cacert.pem"
#define DEF_CERT_FN		"/etc/client.pem"

//ql
//#include "../../boa/src/LINUX/options.h"
//#include <rtk/options.h> keith remove
#ifdef ENABLE_SIGNATURE_ADV
extern int upgrade;
#endif

/*here is the reason why to disable this: the mib-related APIs will handle this*/
#if 0
//xl_yue:close this macro for 8672
//jiunming, this code is shared by 8672 and 8671
#ifndef _LINUX_2_6_
//8671 case
#define __CLOSE_INTERFACE_BEFORE_WRITE_
#endif //_LINUX_2_6_
#endif //if 0

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>


int sendOpertionDoneMsg2MidIntf(char opertion)
{
	int spid;
	FILE * spidfile;
	int msgid;
	int ret;
 	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*sendBuf = OP_OperationDone;	/*Opcode*/
	*(sendBuf+1) = 1;				/*N*/
	*(sendBuf+2) = TYPE_Operation;	/*type*/
	W_WORD2CHAR((sendBuf+3), 1);	/*length*/
	*(sendBuf+5) = opertion;		/*value:'1'- PING;'2'-ATMF5Loopback;'3'-DSL*/

	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		//fprintf(stdout,"get cwmp msgqueue error!\n");
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(MW_INTF_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		//fprintf(stdout,"midware interface pidfile not exists\n");
		return -1;
	}

	sendMsg.msg_type = spid;
	sendMsg.msg_datatype = PACKET_OK;
	if(msgsnd(msgid, (void *)&sendMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"send message to midwareintf error!\n");
		return -1;
	}

 	return 0;
}
#endif

/*********************************************************************/
/* utility */
/*********************************************************************/
int upgradeConfig( char *fwFilename )
{
	//printf("<%s:%d>, filename=%s\n", __FUNCTION__, __LINE__, fwFilename);
	unsigned int filelen;
	unsigned int reboot_Wait=0;
	char cmdBuf[100];
	int ret = -1;
	char buff_msg[200];
	int type=0;
	if(fwFilename == NULL)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		return -1;
	}
	
#if defined(ECOS_MEM_CHAIN_API)
	filelen = fileSize;
#else
	memcpy(&filelen,fwFilename,4); //grap fw length
	fwFilename += 4;
#endif

	//printf("<%s:%d>, filelen=%d\n", __FUNCTION__, __LINE__, filelen);
	if (filelen <= 0)
	{		
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		return -1;
	}	



	reboot_Wait = 50;

	//printf("<%s:%d>Ready to upgrade the new Config. Config length=%u, reboot_Wait=%u \n", __FUNCTION__, __LINE__,filelen,reboot_Wait  ) ;
	//printf("filename=%s, filelen=%d, type=%d\n", fwFilename, filelen, type);
	
	doUpdateConfigIntoFlash(fwFilename, filelen, &type,&ret);
	
	if (ret == 0 || type == 0) { // checksum error
		ret=-1;
	}else{
		sleep(2);
	}
	//printf("<%s:%d>, ret=%d\n", __FUNCTION__,__LINE__, ret);
	return ret;
}

int upgradeFirmware( char *fwFilename )
{
	//diag_printf("enter <%s %d>, fwFilename=%s\n", __FUNCTION__, __LINE__, fwFilename);
	long filelen;
	int reboot_Wait=0;
	char cmdBuf[100];
	int ret = -1;
	char buff_msg[200];
	
	if(fwFilename == NULL)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		return -1;
	}
	
#if defined(ECOS_MEM_CHAIN_API)
	filelen = fileSize;
#else
	memcpy(&filelen,fwFilename,4); //grap fw length
	fwFilename += 4;
#endif

	if (filelen <= 0)
	{		
		CWMPDBG( 0, ( stderr, "<%s:%d>Image file not exist\n", __FUNCTION__, __LINE__ ) );
		return -1;
	}	

#ifdef CHECK_SETTING
	unlink("/var/log/messages");
	va_cmd("/bin/CreatexmlConfig", 0, 1);
	va_cmd("/bin/cp", 2, 1,  CONFIG_XMLFILE, OLD_SETTING_FILE);
	va_cmd( "/bin/flatfsd", 1, 1, "-s" );
#endif

	reboot_Wait = (filelen/30000);

	CWMPDBG( 1, ( stderr, "<%s:%d>Ready to upgrade the new firmware. firmware length=%u, reboot_Wait=%u \n", __FUNCTION__, __LINE__,filelen,reboot_Wait  ) );

	//pre-set watch dog time to reboot device whether F/W success or not.	

//	if(doFirmwareUpgrade(fwFilename, filelen, 0, buff_msg) == 1) // 1:ok
//		ret = 0;

	//diag_printf("out <%s %d>, ret=%d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}


/*********************************************************************/
/* porting functions */
/*********************************************************************/
#if defined(CONFIG_DEV_xDSL)

void port_setuppid(void)
{
	int tr069_pid;
	unsigned char value[32];
	
	tr069_pid = (int)getpid();
	sprintf(value,"%d",tr069_pid);
	
	if (va_cmd("/bin/sarctl",2,1,"tr069_pid",value)){
		//printf("sarctl tr069_pid %s failed\n", value);
    }
	
}	
#else
void port_setuppid(void)
{
	FILE *f;
	int tr069_pid;
	char *pidfile = "/var/run/tr069.pid";

	tr069_pid = (int)getpid();	
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", tr069_pid);
	fclose(f);

}
#endif
int port_init_userdata( struct cwmp_userdata *data )
{
	//diag_printf("enter port_init_userdata\n");
	char buf[256 + 1];
	unsigned int ch;
	unsigned int uVal;

	//diag_printf("[%s:%d]1\n", __FUNCTION__, __LINE__);
	if (data) {

		char acs_url_old[256 + 1];

		memset( data, 0, sizeof( struct cwmp_userdata ) );
		
		//relative to SOAP header
		data->ID = 1;
		data->HoldRequests = 0;
		data->NoMoreRequests = 0;
		data->CPE_MaxEnvelopes = 1;
		data->ACS_MaxEnvelopes = 1;
		
		//cwmp:fault
		data->FaultCode=0;

		//printf("1.1\n");
		//download/upload
		data->DownloadState = DOWNLD_NONE;
		data->DownloadWay = DLWAY_NONE;
		data->DLCommandKey = mib_get(MIB_CWMP_DL_COMMANDKEY,(void *)buf) ? strdup(buf) : NULL;
		data->DLStartTime = mib_get(MIB_CWMP_DL_STARTTIME, (void *)&uVal) ? uVal : 0;
		data->DLCompleteTime = mib_get(MIB_CWMP_DL_COMPLETETIME, (void *)&uVal) ? uVal : 0;
		data->DLFaultCode = mib_get(MIB_CWMP_DL_FAULTCODE, (void *)&uVal) ? uVal : 0;

		//printf("1.2\n");
	    //diag_printf("[%s:%d]1.1\n", __FUNCTION__, __LINE__);
		//inform
		data->InformInterval = mib_get(MIB_CWMP_INFORM_INTERVAL, (void *)&uVal) ? uVal : 60;
		data->InformTime = mib_get(MIB_CWMP_INFORM_TIME, (void *)&uVal) ? uVal : 0;
		data->PeriodicInform = mib_get(MIB_CWMP_INFORM_ENABLE, (void *)&ch) ? ch : 1;
		data->EventCode= mib_get(MIB_CWMP_INFORM_EVENTCODE, (void *)&uVal) ? uVal : 0;
		data->NotifyParameter=NULL;
		data->InformIntervalCnt = 0;

		//printf("1.3\n");
	    //diag_printf("[%s:%d]1.2\n", __FUNCTION__, __LINE__);
		//ScheduleInform
		data->ScheduleInformCnt = 0;
		data->SI_CommandKey = mib_get(MIB_CWMP_SI_COMMANDKEY, (void *)buf) ? strdup(buf) : NULL;

		//printf("1.4\n");
		//Reboot
		data->RB_CommandKey = mib_get(MIB_CWMP_RB_COMMANDKEY, (void *)buf) ? strdup(buf) : NULL;
		data->Reboot = 0;

		//printf("1.5\n");
		//FactoryReset
		data->FactoryReset = 0;

		//printf("1.6\n");
		// andrew
		data->url = mib_get(MIB_CWMP_ACS_URL, (void *)buf) ? strdup(buf) : strdup("");
		mib_get(MIB_CWMP_ACS_URL_OLD, (void*)acs_url_old);

		//printf("1.7\n");
		data->username = mib_get(MIB_CWMP_ACS_USERNAME, (void *)buf) ? strdup(buf) : NULL;
		data->password = mib_get(MIB_CWMP_ACS_PASSWORD, (void *)buf) ? strdup(buf) : NULL;
		//use the wan ip address as realm??
		data->conreq_username = mib_get(MIB_CWMP_CONREQ_USERNAME, (void *)buf) ? strdup(buf) : NULL;
		data->conreq_password = mib_get(MIB_CWMP_CONREQ_PASSWORD, (void *)buf) ? strdup(buf) : NULL;
		data->realm = CWMP_HTTP_REALM;
		data->server_port = mib_get(MIB_CWMP_CONREQ_PORT, (void *)&uVal) ? uVal : 7547;
		//printf("1.8\n");
	    //diag_printf("[%s:%d]data=%p, server_port=%d\n", __FUNCTION__, __LINE__, data, data->server_port);
		if (mib_get(MIB_CWMP_CONREQ_PATH, (void *)&buf[1])) {
			if (buf[1] != '/') {
				buf[0]='/';
				data->server_path = strdup( buf );
			}else
				data->server_path = strdup( &buf[1] );
		}else{
			data->server_path =  strdup("/");
		}
		/*data->machine = &cpe_client;*/
		data->redirect_url = NULL;
		data->redirect_count = 0;

		//certificate
		//printf("1.9\n");
		data->cert_passwd = mib_get( MIB_CWMP_CERT_PASSWORD, (void *)buf ) ? strdup(buf) : NULL;
#if 0
		{
			struct stat file_stat;
			
			if( stat( CERT_FNAME, &file_stat )<0 )
				data->cert_path = strdup(DEF_CERT_FN);
			else
				data->cert_path = strdup(CERT_FNAME);

			if( stat( CA_FNAME, &file_stat )<0 )
				data->ca_cert = strdup(DEF_CA_FN);
			else
				data->ca_cert = strdup(CA_FNAME);
		}
#else
		{
			data->ca_cert = strdup(CA_FNAME);
			data->cert_path = strdup(CERT_FNAME);
		}
#endif
		//printf("data->ca_cert=%s, data->cert_path=%s\n", data->ca_cert, data->cert_path);
		
#ifdef CONFIG_MIDDLEWARE
		//printf("1.10\n");
		mib_get(MIB_CWMP_TR069_ENABLE, &ch);
		if (ch == 0) {	/*midware enabled*/
			data->notify_filename = strdup( MW_NOTIFY_FILENAME );
		}else
#endif
		//printf("1.11\n");
	    //diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
		data->notify_filename = strdup( NOTIFY_FILENAME );
		
		if(strcmp(acs_url_old, data->url) != 0)
		{
			data->EventCode |= 0x00001;//	/*0 BOOTSTRAP*/;
			CWMPDBG( _BOOTSTRAP, ( stderr, "url_changed from [%s] to [%s]. <%s:%d>\n",acs_url_old, data->url, __FUNCTION__, __LINE__ ) );	
		}
		else
			data->url_changed = 0;
		
		/*china-telecom has a extension for inform message, X_OUI_AssocDevice*/
		/*0: diable this field, 1:enable this filed*/
		data->inform_ct_ext = 0;
		//printf("1.12\n");
	    //diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
#ifdef _INFORM_EXT_FOR_X_CT_ 
{ 
		//printf("1.13\n");
			if (mib_get(MIB_CWMP_FLAG, (void *)&ch)) {
				if (ch & CWMP_FLAG_CTINFORMEXT)
					data->inform_ct_ext = 1; 
			}
		}
#endif
#if defined(_PRMT_X_CT_COM_PING_)
		//printf("1.14\n");
		mib_get(MIB_CWMP_CT_PING_ENABLED, (void *)&ch);
		//data->PingConfigState_ct_ext = ch;
#endif
	}
	
	//diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	//printf("out port_init_userdata\n");
	return 0;
}


int port_update_userdata( struct cwmp_userdata *data, int is_discon )
{
	char buf[256 + 1];
	unsigned char ch, *pnew, *pold;
	unsigned int vUint=0;

	if( mib_get(MIB_CWMP_CONREQ_USERNAME, buf)!=0 )
	{
		if( (data->conreq_username==NULL) ||
		    (strcmp(data->conreq_username,buf)!=0) )
		{
			pnew = strdup(buf);
			pold = data->conreq_username;
			data->conreq_username = pnew;
			if(pold) free(pold);
		}
	}
	
	if( mib_get(MIB_CWMP_CONREQ_PASSWORD, buf)!=0 )
	{
		if( (data->conreq_password==NULL) ||
		    (strcmp(data->conreq_password,buf)!=0) )
		{
			pnew = strdup(buf);
			pold = data->conreq_password;
			data->conreq_password = pnew;
			if(pold) free(pold);
		}
	}

//#if defined(FINISH_MAINTENANCE_SUPPORT) || defined(CONFIG_MIDDLEWARE)
	vUint=0;
	if( mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)!=0 )
	{
		if( vUint & EC_X_CT_COM_ACCOUNT )
		{
			fprintf( stderr, "get EC_X_CT_COM_ACCOUNT\n" );
			cwmpSendEvent( EC_X_CT_COM_ACCOUNT );
			vUint = vUint&(~EC_X_CT_COM_ACCOUNT);
			mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
//#endif
#ifdef  _PRMT_X_CT_COM_USERINFO_
	vUint=0;
	if( mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)!=0 )
	{
		if( vUint & EC_X_CT_COM_BIND )
		{
			fprintf( stderr, "get EC_X_CT_COM_BIND\n" );
			cwmpSendEvent( EC_X_CT_COM_BIND );
			vUint = vUint&(~EC_X_CT_COM_BIND);
			mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
#endif

#ifdef E8B_NEW_DIAGNOSE
	vUint = 0;
	if (mib_get(MIB_CWMP_INFORM_EVENTCODE, &vUint)!=0) {
		if (vUint & EC_X_CT_COM_SEND_INFORM) {
			fprintf(stderr, "get EC_X_CT_COM_SEND_INFORM\n");
			cwmpSendEvent(EC_X_CT_COM_SEND_INFORM);
			vUint = vUint & (~EC_X_CT_COM_SEND_INFORM);
			mib_set(MIB_CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
#endif

	//update the acs url/username/password only when the connection is disconnected
	if(is_discon)
	{
		if( mib_get(MIB_CWMP_ACS_URL, buf)!=0 )
		{
			if( (data->url==NULL) ||
			    (strcmp(data->url,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->url;
				data->url = pnew;
				if(pold) free(pold);
				//reset something??
				data->url_changed = 1;
			}
		}
	
		if( mib_get(MIB_CWMP_ACS_USERNAME, buf)!=0 )
		{
			if( (data->username==NULL) ||
			    (strcmp(data->username,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->username;
				data->username = pnew;
				if(pold) free(pold);
			}
		}
	
		if( mib_get(MIB_CWMP_ACS_PASSWORD, buf)!=0 )
		{
			if( (data->password==NULL) ||
			    (strcmp(data->password,buf)!=0) )
			{
				pnew = strdup(buf);
				pold = data->password;
				data->password = pnew;
				if(pold) free(pold);
			}
		}

/*star:20100112 START add to check if these three parameters changed*/
		{
			unsigned char vchar;
			unsigned int vint1,vint2,vint3;
			
			mib_get(MIB_CWMP_INFORM_ENABLE, &vint3);
			mib_get(MIB_CWMP_INFORM_INTERVAL,&vint1);
			mib_get(MIB_CWMP_INFORM_TIME,&vint2);
			if( data->PeriodicInform!=vint3 ||
			    data->InformInterval!=vint1 ||
			    data->InformTime!=vint2 )
			{
				//printf("update the informinterval!\n" );
				data->PeriodicInform = vint3;
				data->InformInterval = vint1;
				data->InformTime = vint2;
				cwmpMgmtSrvInformInterval();
			}
		}
/*star:20100112 END*/

	}
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	if(getTr069QosFlag()==0){
		unsigned char buf[256+1]={0};
		unsigned char acsurl[256+1]={0};
		if(MgmtSrvGetConReqURL(buf, 256)){
			if(getOldACS(acsurl))
				setQosfortr069(1,acsurl);
			acsurl[0]=0;
			if(mib_get(MIB_CWMP_ACS_URL,acsurl)){
				setQosfortr069(1,acsurl);
				setQosfortr069(0,acsurl);
			}
			setTr069QosFlag(1);
		}
	}
#endif
/*star:20100305 END*/
	return 0;
}

void port_save_reboot( struct cwmp_userdata *user, int reboot_flag )
{
	//printf("enter port_save_reboot\n");
	char acs_url[256 + 1];



	if( user )
	{
		//reboot commandkey
		if(user->RB_CommandKey)
			mib_set( MIB_CWMP_RB_COMMANDKEY, user->RB_CommandKey );
		else
			mib_set( MIB_CWMP_RB_COMMANDKEY, "" );

		//scheduleinform commandkey
		if(user->SI_CommandKey)
			mib_set( MIB_CWMP_SI_COMMANDKEY, user->SI_CommandKey );
		else
			mib_set( MIB_CWMP_SI_COMMANDKEY, "" );
	
		//related to download
		if(user->DLCommandKey)
			mib_set( MIB_CWMP_DL_COMMANDKEY, user->DLCommandKey );
		else
			mib_set( MIB_CWMP_DL_COMMANDKEY, "" );
				
		mib_set( MIB_CWMP_DL_STARTTIME, &user->DLStartTime );
		mib_set( MIB_CWMP_DL_COMPLETETIME, &user->DLCompleteTime );
		mib_set( MIB_CWMP_DL_FAULTCODE, &user->DLFaultCode );

		//inform
		mib_set( MIB_CWMP_INFORM_EVENTCODE, &user->EventCode );

		mib_get(MIB_CWMP_ACS_URL, (void*)acs_url);
		mib_set(MIB_CWMP_ACS_URL_OLD, (void*)acs_url);
		user->url_changed = 0;
	}


	//printf("<%s:%d>\n", __FUNCTION__, __LINE__ );//fflush(stderr);
	apmib_update(CURRENT_SETTING);
	//printf("<%s:%d>\n", __FUNCTION__, __LINE__ );//fflush(stderr);

	if(reboot_flag)
	{
		CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
		cmd_reboot();
		exit(0);
	}

}

void port_factoryreset_reboot(void)
{
	//printf("enter port_factoryreset_reboot\n");
	CWMPDBG( 3, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	// Commented by Mason Yu. for not use default setting
	//mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
	//va_cmd("/bin/flash", 2, 1, "default", "ds");
	
#if defined(CONFIG_DEV_xDSL	)
#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef	RESERVE_KEY_SETTING
	reserve_critical_setting(1);
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#else
	va_cmd("/bin/flash", 3, 1, "default", "cs", "tr69");
#endif
#else
#ifdef	RESERVE_KEY_SETTING
	reserve_critical_setting();
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#else
	va_cmd("/bin/flash", 2, 1, "default", "cs");
#endif
#endif

#else
	va_cmd("flash", 1, 1, "reset");
#endif

	CWMPDBG( 0, ( stderr, "<%s:%d>The system is restarting ...\n", __FUNCTION__, __LINE__ ) );
#ifdef CONFIG_BOA_WEB_E8B_CH
	sleep(5); //wait finishing setdefault configuration for 16M flash
#endif
	cmd_reboot();
	exit(0);
}

int port_before_download( int file_type, char *target )
{
	//diag_printf( "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	if(target==NULL) return -1;

	switch( file_type )
	{
	case DLTYPE_IMAGE:
		strcpy( target, "/tmp/vm.img" );
		break;
//	case DLTYPE_WEB: //not support right now
//		strcpy( target, "/tmp/web.bin" );
//		break;
	case DLTYPE_CONFIG:	
		strcpy( target, "/tmp/config.xml" );
		break;
	}
//ql
#if defined(CONFIG_DEV_xDSL)
#ifndef CONFIG_MIDDLEWARE
#ifndef ENABLE_SIGNATURE_ADV
	cmd_killproc(ALL_PID & ~(1<<PID_CWMP));	
#endif
#endif
#endif
	return 0;
}

int port_after_download( int file_type, char *target )
{
	switch(file_type)
	{
	case DLTYPE_IMAGE:
		//update firmware
#ifdef ENABLE_SIGNATURE_ADV
		if (upgrade == 2) {
#endif
			if( upgradeFirmware( target ) ) //return 0: success
			{
				CWMPDBG( 0, ( stderr, "<%s:%d>Image Checksum Failed!\n", __FUNCTION__, __LINE__ ) );
				return ERR_9010; 
			}
#ifdef ENABLE_SIGNATURE_ADV
		}else{
			CWMPDBG( 0, ( stderr, "<%s:%d>upgrade!=2!\n", __FUNCTION__, __LINE__ ) );
			return ERR_9010; 
		}
#endif
		break;
	case DLTYPE_CONFIG:
		//update config
#if !defined(CONFIG_DEV_xDSL)		
		if(upgradeConfig(target) < 0 ){
			diag_printf("<%s:%d>Config Checksum Failed!\n", __FUNCTION__, __LINE__ ) ;
			return ERR_9010; 
		}
		
#else
		if( va_cmd("/bin/loadconfig",0,1) )
		{
			fprintf( stderr, "exec /bin/loadconfig error!\n" );
			return ERR_9002; 
		}
#ifdef CONFIG_BOA_WEB_E8B_CH
		FILE *fpin;
		int filelen;
		fpin=fopen(target,"r");
		if(fpin>0){
			fseek(fpin, 0, SEEK_END);
			filelen = ftell(fpin);
			fseek(fpin, 0, SEEK_SET);
			fclose(fpin);
			if(filelen<=0)
				return ERR_9002;
		}else
			return ERR_9002;
#endif
#endif
		break;
	} //end switch

	return 0;
}


int port_before_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	switch(file_type)
	{
	case DLTYPE_CONFIG:
#if !defined(CONFIG_DEV_xDSL)
		strcpy( target, "/web/config.dat" );
#else
                strcpy( target, "/tmp/config.xml" );
		//if( va_cmd("/bin/CreatexmlConfig",0,1) )
		if( va_cmd("/bin/saveconfig",0,1) )
		{
			fprintf( stderr, "exec /bin/saveconfig error!\n" );
		}
#endif
		break;
	case DLTYPE_LOG:
		//ifndef CONFIG_USER_BUSYBOX_SYSLOGD, send empty http put(content-length=0)
#ifdef CONFIG_BOA_WEB_E8B_CH
		strcpy( target, "/var/config/syslogd.txt" );
#else
			strcpy( target, "/var/log/messages" );
#endif
		break;
	}

	fprintf( stderr, "<%s:%d> file type:%d, target:%s\n", __FUNCTION__, __LINE__, file_type, target?target:"" );

	return 0;
}


int port_after_upload( int file_type, char *target )
{
	if( target==NULL ) return -1;

	//remove the target file
	switch(file_type)
	{
	case DLTYPE_CONFIG:
	#ifdef CONFIG_DEV_xDSL	
		if( strlen(target) )	remove( target );
	#endif		
		break;
	case DLTYPE_LOG:
		//not have to remove the log file
		break;
	}

	return 0;	
}

int port_notify_save( char *name )
{
#if 0
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		CWMPDBG( 0, ( stderr, "<%s:%d>exec 'flatfsd -s' error!\n", __FUNCTION__, __LINE__ ) );
#else
#endif

	return 0;
}

int port_session_closed(struct cwmp_userdata *data)
{
	unsigned int delay=3;

	if (gStartPing)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		//printf("=================================================START PING=============================================\n");
		cwmpStartPingDiag();
		gStartPing = 0;
	}

	else if(!gCTStartPing){
		gCTStartPing=1;
	}

	
#ifdef CONFIG_DEV_xDSL
	else if(gStartDSLDiag)
	{
		cwmpSetCpeHold( 1 );
		cwmpStartDSLDiag();

		gStartDSLDiag=0;
	}

	else if(gStartATMF5LB)
	{
		cwmpStartATMF5LB();
		gStartATMF5LB=0;
	}
#endif

#ifdef _PRMT_TR143_
	else if(gStartTR143DownloadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
	
		StartTR143DownloadDiag();
		gStartTR143DownloadDiag=0;
	}else if(gStartTR143UploadDiag)
	{
#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTR143UploadDiag();
		gStartTR143UploadDiag=0;
	}
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_  
  else if(gStartTraceRouteDiag)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		StartTraceRouteDiag();
		gStartTraceRouteDiag=0;
  }
#endif //_SUPPORT_TRACEROUTE_PROFILE_
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_  
  else if(gStartReset)
	{
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
		/*delay some seconds to ready*/
		{while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
		cwmpStartReset();
		gStartReset=0;
	}
#endif
/*ping_zhang:20081217 END*/	

/*alex_hung:20110923 support TR-104, CONFIG_APP_TR104*/
#if 0 //def CONFIG_APP_TR104
        else if(gVoipReq)
        {
	#ifdef __CLOSE_INTERFACE_BEFORE_WRITE_
            /*delay some seconds to ready*/
            {while(delay!=0) delay=sleep(delay);}
	#endif /*__CLOSE_INTERFACE_BEFORE_WRITE_*/
            cwmpStartVoipDiag();
        }
#endif /*CONFIG_APP_TR104 END*/

#ifdef CONFIG_USE_XML
	unlink(CONFIG_FILE_NAME);
#endif	

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unlink(FILE4CaptivePortal);
#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
	unlink(FILE4DHCPReservedIPAddr);
#endif //SUPPORT_DHCP_RESERVED_IPADDR
	
	//update the userdata
	
	return 0;
}

int port_backup_config( void )
{
	int ret=0;
	CWMPDBG( _SET_ATTR, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	
	ret = mib_backup(1);
	
	CWMPDBG( _SET_ATTR, ( stderr, "<%s:%d> ret=%d\n", __FUNCTION__, __LINE__,ret ) );
	
	return ret;
}

int port_restore_config(int restore)
{
	int ret=0;
	CWMPDBG( _SET_ATTR, ( stderr, "<%s:%d> \n", __FUNCTION__, __LINE__) );

	ret=mib_restore(restore);	
	CWMPDBG( _SET_ATTR, ( stderr, "<%s:%d> ret=%d\n", __FUNCTION__, __LINE__,ret ) );

	return ret;
}

#ifdef CONFIG_BOA_WEB_E8B_CH
int isTR069(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	WANIFACE_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);
	for(i=1;i<=total;i++){
		
		*((char *)&Entry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)&Entry))
			continue;
		
		if(Entry.ConDevInstNum==devnum 
			&& Entry.ConIPInstNum==ipnum 
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype==APPTYPE_TR069_INTERNET || Entry.applicationtype==APPTYPE_TR069)){
			return 1;
		}	
	}

	return 0;

}
int isINTERNET(char *name)
{
	unsigned int devnum,ipnum,pppnum;
	WANIFACE_T Entry;
	unsigned int total;
	int i;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );

	mib_get(MIB_WANIFACE_TBL_NUM, (void *)&total);

	for(i=1;i<=total;i++){
		*((char *)&Entry) = (char)i;
		if(!mib_get(MIB_WANIFACE_TBL, (void *)&Entry))
			continue;
		
		if(Entry.ConDevInstNum==devnum 
			&& Entry.ConIPInstNum==ipnum 
			&& Entry.ConPPPInstNum==pppnum
			&& (Entry.applicationtype==APPTYPE_TR069_INTERNET || Entry.applicationtype==APPTYPE_INTERNET)){
			return 1;
		}	
	}

	return 0;


}

#ifdef _PRMT_X_CT_COM_USERINFO_
void resetuserinfo(void)
{
	unsigned char regstatus = 1;
	unsigned int regtimes = 0;

	mib_set(MIB_CWMP_USERINFO_STATUS, &regstatus);
	mib_set(MIB_CWMP_USERINFO_TIMES, &regtimes);
}
#endif




#ifdef _PRMT_X_CT_COM_PING_



static void *CTipPingConfig_thread(void *arg) 
{
	int PingConfigEnabled=0;
	int EntryNum=0;
	int PingResult=0;
	
	for (;;) {


		
		PingResult=checkCTPing();

		
		mib_get( MIB_CWMP_CT_PING_ENABLED, (void *)&PingConfigEnabled);
		mib_get( MIB_CWMP_CT_PING_TBL_NUM, (void *)&EntryNum);
		if(PingConfigEnabled > 0 && EntryNum > 0){
			sleep(1);
			PingConfigEnabled =0;
			EntryNum=0;
		}else{
		CWMPDBG( 1, ( stderr, "<%s:%d>exit PingConfig thread \n", __FUNCTION__, __LINE__) );
			break;
		}
	
	}


	
}

void cwmpStartCTPingConfigDiag(struct cwmp_userdata *ud) 
{
	
	pthread_t ping_pid;
	if( pthread_create( &ping_pid, NULL, CTipPingConfig_thread, 0 ) != 0 )
	{
		CWMPDBG( 1, ( stderr, "<%s:%d>create PingConfig thread Fail \n", __FUNCTION__, __LINE__) );
		ud->PingConfigState_ct_ext = PING_STOPPED;
		return;
	}
	pthread_detach(ping_pid);
	ud->PingConfigState_ct_ext = PING_RUNNING;
	//CWMPDBG( 1, ( stderr, "<%s:%d>PingConfig RUNNING \n", __FUNCTION__, __LINE__) );
}

#endif





#endif

