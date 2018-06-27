#ifndef _PRMT_LANDEVICE_WLAN_H_
#define _PRMT_LANDEVICE_WLAN_H_

#include "prmt_igd.h"

#include "prmt_utility.h" //keith add.

#ifdef WLAN_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif



	#if defined(MOD_FOR_TR098_DUALBAND)

#if defined(MBSSID) && defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	12
#elif defined(MBSSID) && !defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	10
#elif !defined(MBSSID) && defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	4
	#else
	#define WLAN_IF_NUM	2
	#endif

#else
#if defined(MBSSID) && defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	6
#elif defined(MBSSID) && !defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	5
#elif !defined(MBSSID) && defined(UNIVERSAL_REPEATER)
	#define WLAN_IF_NUM	2
#else
	#define WLAN_IF_NUM	1
#endif

#endif


extern struct CWMP_LEAF tPreSharedKeyEntityLeaf[];
extern struct CWMP_NODE tPreSharedKeyObject[];
extern struct CWMP_LEAF tWEPKeyEntityLeaf[];
extern struct CWMP_NODE tWEPKeyObject[];
extern struct CWMP_LEAF tAscDeviceEntityLeaf[];
extern struct CWMP_LINKNODE tAscDeviceObject[];
extern struct CWMP_LEAF tWLANConfEntityLeaf[];
extern struct CWMP_NODE tWLANConfEntityObject[];
extern struct CWMP_NODE tWLANConfigObject[];

int getPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getAscDeviceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objAscDevice(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data);

#if 1//def CTCOM_WLAN_REQ
int objWLANConfiguration(char *name, struct CWMP_LEAF *e, int type, void *data);
#endif

#ifdef __cplusplus
}
#endif
#endif /*#ifdef WLAN_SUPPORT*/
#endif /*_PRMT_LANDEVICE_WLAN_H_*/
