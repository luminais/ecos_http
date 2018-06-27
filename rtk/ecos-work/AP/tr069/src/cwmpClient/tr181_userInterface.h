#ifndef _TR181_USERINTERFACE_H_
#define _TR181_USERINTERFACE_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_LEAF tUserInterfaceLeaf[];

int getUserInterface(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setUserInterface(char *name, struct CWMP_LEAF *entity, int type, void *data);
	
#ifdef __cplusplus
}
#endif

#endif /*_TR181_USERINTERFACE_H_*/
