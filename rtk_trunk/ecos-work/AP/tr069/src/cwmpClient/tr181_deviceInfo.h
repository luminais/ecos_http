#ifndef _TR181_DEVICE_INFO_H_
#define _TR181_DEVICE_INFO_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDevInfoLeaf[];

int getDevInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDevInfo(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DEVICE_INFO_H_*/
