#ifndef _TR181_WIFI_RADIO_H_
#define _TR181_WIFI_RADIO_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tWifiObject[];

int objWifiAPAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data);

int getWifiAPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWifiAPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


int getWifiSsidEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWifiSsidEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWifiRadioEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWifiRadioEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWifiRaStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int getWifiSsidStats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWifiAPSecurity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWifiAPSecurity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWifiAPWps(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWifiAPWps(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWifiAPAssocats(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_WIFI_RADIO_H_*/

