#ifndef _TR181_DHCPV4_H_
#define _TR181_DHCPV4_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tDhcpv4Leaf[];

int getDhcpv4Info(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DHCPV4_H_*/

