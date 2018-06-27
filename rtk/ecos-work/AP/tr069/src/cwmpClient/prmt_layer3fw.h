#ifndef _PRMT_LAYER3FW_H_
#define _PRMT_LAYER3FW_H_

#include "prmt_igd.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tForwardingEntityLeaf[];
extern struct CWMP_LINKNODE tForwardingObject[];
extern struct CWMP_LEAF tLayer3ForwardingLeaf[];
extern struct CWMP_NODE tLayer3ForwardingObject[];

extern struct sCWMP_ENTITY tForwardingEntity[];
extern struct sCWMP_ENTITY tForwarding[];
extern struct sCWMP_ENTITY tLayer3Forwarding[];

int getFwEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setFwEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getLayer3Fw(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLayer3Fw(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objForwading(char *name, struct CWMP_LEAF *entity, int type, void *data);

//by cairui ,temp
typedef struct CWMP_LEAF MIB_CE_IP_ROUTE_T;
#ifdef __cplusplus
}
#endif

#endif /*_PRMT_LAYER3FW_H_*/
