#ifndef _PRMT_DEVICEINFO_H_
#define _PRMT_DEVICEINFO_H_
#include <string.h>

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDeviceInfoLeaf[];
extern struct CWMP_NODE tDeviceInfoObject[];
extern struct CWMP_NODE tVendorConfigObject[];
extern struct CWMP_PRMT tVendorCfgEntityLeafInfo[];
extern int gCTStartPing;

int getDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getVendorCfgEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef _PRMT_DEVICECONFIG_
extern struct CWMP_LEAF tDeviceConfigLeaf[];
int getDeviceConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDeviceConfig(char *name, struct CWMP_LEAF *entity, int type, void *data);
#endif //_PRMT_DEVICECONFIG_

#define CONFIG_FILE_NAME "/tmp/tr69cfg.xml" 


#ifdef __cplusplus
}
#endif

#endif /*_PRMT_DEVICEINFO_H_*/
