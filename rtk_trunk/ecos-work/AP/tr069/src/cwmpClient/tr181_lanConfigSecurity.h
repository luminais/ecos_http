#ifndef _TR181_LAN_CONFIG_SECURITY_H_
#define _TR181_LAN_CONFIG_SECURITY_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tLANCfgSecurityLeaf[];

int getLANCfgSecurityInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANCfgSecurityInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_LAN_CONFIG_SECURITY_H_*/



