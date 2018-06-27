#ifndef _TR181_BRIDGING_H_
#define _TR181_BRIDGING_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tBridgingLeaf[];

int getBridgingInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_BRIDGING_H_*/
