#ifndef _TR181_DNS_H_
#define _TR181_DNS_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_IPV6
extern struct CWMP_LEAF tDNSLeaf[];

int getDNSInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif // CONFIG_IPV6

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DNS_H_*/


