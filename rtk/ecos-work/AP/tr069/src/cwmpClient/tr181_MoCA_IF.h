#ifndef _TR181_MOCA_IF_H_
#define _TR181_MOCA_IF_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tMoCAIFObject[];


int objMoCAIF(char *name, struct CWMP_LEAF *e, int type, void *data);

int getMoCAIFEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setMoCAIFEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getMoCAIFStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getMoCAIFQoS(char *name, struct CWMP_LEAF *entity, int *type, void **data);


int objMoCAIFQoSFlowStats(char *name, struct CWMP_LEAF *e, int type, void *data);

int getMoCAIFQoSFlowStEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int objMoCAIFAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data);

int getMoCAIFAssocDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_MOCA_IF_H_*/
