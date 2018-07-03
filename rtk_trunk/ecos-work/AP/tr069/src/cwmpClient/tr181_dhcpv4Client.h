#ifndef _TR181_DHCPV4_CLIENT_H_
#define _TR181_DHCPV4_CLIENT_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tDhcpv4ClientObject[];

int getDhcpv4ClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDhcpv4ClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);


int objDhcpv4ClientSent(char *name, struct CWMP_LEAF *e, int type, void *data);
int objDhcpv4ClientReq(char *name, struct CWMP_LEAF *e, int type, void *data);

int getDhcpv4ClientSentEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDhcpv4ClientSentEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getDhcpv4ClientReqEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDhcpv4ClientReqEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DHCPV4_CLIENT_H_*/

