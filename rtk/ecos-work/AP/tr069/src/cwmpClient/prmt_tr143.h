#ifndef _PRMT_TR143_H_
#define _PRMT_TR143_H_

#include "prmt_igd.h"
#include <sys/time.h>
#include "prmt_utility.h" ///Brad add for TR143
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _PRMT_TR143_

extern struct CWMP_LEAF tPerformanceDiagnosticLeaf[];
int getPerformanceDiagnostic(char *name, struct CWMP_LEAF *entity, int *type, void **data);

extern struct CWMP_LEAF tDownloadDiagnosticsLeaf[];
int getDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tUploadDiagnosticsLeaf[];
int getUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern struct CWMP_LEAF tUDPEchoConfigLeaf[];
int getUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int type, void *data);


#ifdef CONFIG_USER_FTP_FTP_FTP
void checkPidforFTPDiag( pid_t  pid );
#endif //CONFIG_USER_FTP_FTP_FTP

extern int gStartTR143DownloadDiag;
extern int gStartTR143UploadDiag;
void StopTR143DownloadDiag(void);
void StopTR143UploadDiag(void);
void StartTR143DownloadDiag(void);
void StartTR143UploadDiag(void);

#endif /*_PRMT_TR143_*/	
#ifdef __cplusplus
}
#endif
#endif /*_PRMT_TR143_H_*/
