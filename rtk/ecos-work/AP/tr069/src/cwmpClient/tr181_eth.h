#ifndef _TR181_ETH_H_
#define _TR181_ETH_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERNET_NUM			5	/* 5 instances of LINK */
#define LINK_NUM				2	/* 2 instances of LINK */


extern struct CWMP_LEAF tEthLeaf[];

int getEthInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_ETH_H_*/
