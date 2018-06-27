#ifndef _LIBCWMP_H_
#define _LIBCWMP_H_

#include "parameter_api.h"
#include "cwmp_utility.h"
#include "cwmp_download.h"
//#include "apmib.h"
#define _USE_FILE_FOR_OUTPUT_ 
#ifdef _USE_FILE_FOR_OUTPUT_
#define OUTXMLFILENAME 		"/tmp/cwmp.xml"
#define OUTXMLFILENAME_BK	"/tmp/cwmp_bk.xml"
#endif


struct cwmp_userdata
{
	//relative to SOAP header
	unsigned int		ID;
	unsigned int		HoldRequests;
	unsigned int		NoMoreRequests;
	unsigned int		CPE_MaxEnvelopes;
	unsigned int		ACS_MaxEnvelopes;
	
	//cwmp:fault
	int			FaultCode;
	
	//download/upload
	int			DownloadState;
	int			DownloadWay;
	char			*DLCommandKey;
	time_t			DLStartTime;
	time_t			DLCompleteTime;
	unsigned int		DLFaultCode;
        DownloadInfo_T		DownloadInfo;
        
        	
	//inform
	unsigned int		InformInterval; //PeriodicInformInterval
	time_t			InformTime; //PeriodicInformTime
	int			PeriodicInform;
	unsigned int		EventCode;
	struct node		*NotifyParameter;
	unsigned int		InformIntervalCnt; 
	
	//ScheduleInform
	unsigned int		ScheduleInformCnt; //for scheduleInform RPC Method, save the DelaySeconds
	char			*SI_CommandKey;

	//Reboot
	char			*RB_CommandKey;	//reboot's commandkey
	int			Reboot; // reboot flag
	
	//FactoryReset
	int			FactoryReset;

	// andrew. 
	char 			*url;	// ACS URL
	char 			*username; // username used to auth us to ACS.
	char 			*password; // passwrd used to auth us to ACS.
	char 			*conreq_username;
	char 			*conreq_password;
	char 			*realm;
	int			server_port;
	char			*server_path;
	void *			machine;
	
	char			*redirect_url;
	int			redirect_count;
	
	//certificate
	char			*cert_passwd;
	char			*cert_path;
	char			*ca_cert;

	char			*notify_filename;
	
	int			url_changed;
	
	int			inform_ct_ext;
	int			PingConfigState_ct_ext;
};


int cwmp_main( struct CWMP_NODE troot[] );

#endif /*#ifndef _LIBCWMP_H_*/
