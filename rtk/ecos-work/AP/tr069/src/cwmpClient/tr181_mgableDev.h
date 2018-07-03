#ifndef _TR181_MGABLEDEV_H_
#define _TR181_MGABLEDEV_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TR069_ANNEX_F_DEVICE_FILE	"/var/udhcpd/tr069_annex_f_device.dat"

extern int gMgableDevNum;
extern struct CWMP_NODE tMgableDevObject[];


int objMgableDev(char *name, struct CWMP_LEAF *e, int type, void *data);

int getMgableDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setMgableDevEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_MGABLEDEV_H_*/
