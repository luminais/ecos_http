#ifndef _TR181_DEVICE_INFO_VENDOR_LOG_FILE_H_
#define _TR181_DEVICE_INFO_VENDOR_LOG_FILE_H_

#include "tr181_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct CWMP_NODE tVendorLogFileObject[];

int getVendorLogFile(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#ifdef __cplusplus
}
#endif

#endif /*_TR181_DEVICE_INFO_VENDOR_LOG_FILE_H_*/

