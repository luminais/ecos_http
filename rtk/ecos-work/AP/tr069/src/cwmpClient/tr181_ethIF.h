#ifndef _TR181_ETHIF_H_
#define _TR181_ETHIF_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tEthIFObject[];

int objEthIF(char *name, struct CWMP_LEAF *e, int type, void *data);

int getEthIFEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setEthIFEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getEthIFStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


int objEthLink(char *name, struct CWMP_LEAF *e, int type, void *data);

int getEthLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setEthLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getEthLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_ETHIF_H_*/
