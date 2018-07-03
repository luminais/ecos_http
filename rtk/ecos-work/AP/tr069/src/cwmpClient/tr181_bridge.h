#ifndef _TR181_BRIDGE_H_
#define _TR181_BRIDGE_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tBrObject[];

int getBrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getBrPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBrPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getBrVlanEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBrVlanEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getBrVlanPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setBrVlanPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getBrPortStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif

#endif /*_TR181_BRIDGE_H_*/
