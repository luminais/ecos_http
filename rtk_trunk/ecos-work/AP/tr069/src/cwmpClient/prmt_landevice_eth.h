#ifndef _PRMT_LANDEVICE_ETH_H_
#define _PRMT_LANDEVICE_ETH_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_EXT_SWITCH
#define CWMP_LANETHIFNO		4
#else
#define CWMP_LANETHIFNO		1
#endif //CONFIG_EXT_SWITCH

extern struct CWMP_NODE tLANEthConfObject[];
int getLANEthConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setLANEthConf(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getLANEthStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);


#ifdef __cplusplus
}
#endif
#endif /*_PRMT_LANDEVICE_ETH_H_*/
