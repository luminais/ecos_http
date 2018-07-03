#ifndef _PRMT_WANATMF5LOOPBACK_H_
#define _PRMT_WANATMF5LOOPBACK_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif


extern struct CWMP_LEAF tWANATMF5LBLeaf[];

int getWANATMF5LB(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANATMF5LB(char *name, struct CWMP_LEAF *entity, int type, void *data);

extern int gStartATMF5LB;
void cwmpStartATMF5LB(void);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANATMF5LOOPBACK_H_*/
