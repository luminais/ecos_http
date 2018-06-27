#ifndef _TR181_DHCPV6_H_
#define _TR181_DHCPV6_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_IPV6
extern struct CWMP_LEAF tDhcpv6Leaf[];

int getDhcpv6Info(char *name, struct CWMP_LEAF *entity, int *type, void **data);
#endif // CONFIG_IPV6

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DHCPV6_H_*/

