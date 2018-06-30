#ifndef _PRMT_WANDEVICE_H_
#define _PRMT_WANDEVICE_H_

#include "prmt_igd.h"
//#include "parameter_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _SUPPORT_ADSL2WAN_PROFILE_
extern struct  CWMP_LEAF tWANDSLIFTestParamsLeaf[];
int getWANDSLIFTestParams(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif //_SUPPORT_ADSL2WAN_PROFILE_

extern struct CWMP_NODE tWANDeviceObject[];
extern struct CWMP_NODE tWANDevEntityObject[];
extern struct CWMP_LEAF tWANDevEntityLeaf[];
extern struct CWMP_LEAF tWANCmnIfCfgLeaf[];

int getWANDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANCmnIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANDSLIfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANEthInfCfg(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANEthStatsLeaf(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_PRMT_WANDEVICE_H_*/
