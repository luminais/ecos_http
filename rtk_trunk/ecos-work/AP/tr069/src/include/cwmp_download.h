#ifndef _CWMP_DOWNLOAD_H_
#define _CWMP_DOWNLOAD_H_

#include "soapH.h"

#ifdef __cplusplus
extern "C" {
#endif


/*download way*/
#define DLWAY_NONE	0
#define DLWAY_DOWN	1
#define DLWAY_UP	2
/*download type*/
#define DLTYPE_NONE	0
#define DLTYPE_IMAGE	1
#define DLTYPE_WEB	2
#define DLTYPE_CONFIG	3
#define DLTYPE_LOG	4
/*download status*/
#define DOWNLD_NONE	0
#define DOWNLD_READY	1
#define DOWNLD_START	2
#define DOWNLD_FINISH	3
#define DOWNLD_ERROR	4
/*download union*/
typedef union
{
struct cwmp__Download	Download;
struct cwmp__Upload	Upload;
}DownloadInfo_T;


extern int gStartDownload;

void cwmpStartDownload(struct soap *soap);






/* TR143 HTTP Download/Upload Diagnostics*/
enum
{
	eTR143_None=0,
	eTR143_Requested,
	eTR143_Completed,
	eTR143_Error_InitConnectionFailed,
	eTR143_Error_NoResponse,
	eTR143_Error_PasswordRequestFailed,
	eTR143_Error_LoginFailed,
	eTR143_Error_NoTransferMode,
	eTR143_Error_NoPASV,
	//download
	eTR143_Error_TransferFailed,
	eTR143_Error_IncorrectSize,
	eTR143_Error_Timeout,
	//upload
	eTR143_Error_NoCWD,
	eTR143_Error_NoSTOR,
	eTR143_Error_NoTransferComplete,

	eTR143_End /*last one*/
};

struct TR143_Diagnostics
{
	int		Way;
	
	int		DiagnosticsState;
	char		*pInterface;
	char		IfName[32];
	char		*pURL;
	unsigned int	DSCP;
	unsigned int	EthernetPriority;
	unsigned int	TestFileLength;
	struct timeval	ROMTime;
	struct timeval	BOMTime;
	struct timeval	EOMTime;
	unsigned int	TestBytesReceived;
	unsigned int	TotalBytesReceived;
	unsigned int	TotalBytesSent;
	struct timeval	TCPOpenRequestTime;
	struct timeval	TCPOpenResponseTime;
	
	unsigned long int	http_pid;
	unsigned long int	ftp_pid;
};

int TR143StartHttpDiag(struct TR143_Diagnostics *p);
int TR143StopHttpDiag(struct TR143_Diagnostics *p);
/* End TR143 HTTP Download/Upload Diagnostics*/






#ifdef __cplusplus
}
#endif

#endif /*_CWMP_DOWNLOAD_H_*/
