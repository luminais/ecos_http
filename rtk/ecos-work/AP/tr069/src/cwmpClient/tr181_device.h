#ifndef _TR181_DEVICE_H_
#define _TR181_DEVICE_H_

#include "libcwmp.h"
#include "cwmp_porting.h"
#include "apmib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	TR181_DEBUG
#define	TR181_TRACE
#define	TR181_ERROR
#define	TR181_WARNING

#ifdef TR181_DEBUG
#define tr181_printf(fmt, arg...) fprintf(stderr, "[TR181][FILE]%s:[FUNCTION]%s:[LINE]%d - "fmt"\n",__FILE__,__FUNCTION__,__LINE__,##arg)
#else
#define tr181_printf(a,...) do{}while(0)
#endif

#ifdef TR181_TRACE
#define tr181_trace() fprintf(stderr, "[TR181][FILE]%s:[FUNCTION]%s:[LINE]%d\n",__FILE__,__FUNCTION__,__LINE__)
#else
#define tr181_trace(a,...) do{}while(0)
#endif

#ifdef TR181_ERROR
#define tr181_error(fmt, arg...) fprintf(stderr, "[TR181][ERROR][%d]"fmt"\n",__LINE__,##arg)
#else
#define tr181_error(a,...) do{}while(0)
#endif

#ifdef TR181_WARNING
#define tr181_warning(fmt, arg...) fprintf(stderr, "[TR181][WARNING][%d]"fmt"\n",__LINE__,##arg)
#else
#define tr181_warning(a,...) do{}while(0)
#endif

#define IF_NAME_SIZE        16

#define _PATH_ETH_PORT_IF_		"/proc/rtl865x/tr181_eth_if"
#define _PATH_ETH_LINK_			"/proc/rtl865x/tr181_eth_link"
#define _PATH_ETH_PORT_STATUS_	"/proc/rtl865x/tr181_eth_stats"

struct eth_port_if {
	char enable[6]; // true, false
	char status[5]; // Up, Down
	unsigned int lastChange;
	char upStream[6]; // true, false
	unsigned int maxBitRate; // 10, 100, 100
	char duplexMode[5]; // Full, Half
};

struct eth_link_if {
	char enable[6];			// true, false
	char status[5];			// Up, Down
	char name[5];			// eth0, eth1
	unsigned int LastChange;
	char MACAddress[20];
};

extern struct CWMP_LEAF tDevLeaf[];
extern struct CWMP_NODE tDevObject[];
extern struct CWMP_NODE tDevROOT[];

extern unsigned int getInstanceNum(char *name, char *objname);
extern char *get_name(char *name, char *p);
extern int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats);
extern int get_eth_port_if_fields(char *bp, struct eth_port_if *pPortIf);
extern int get_eth_link_fields(char *bp, struct eth_link_if *pLinkIf);
extern int getEthPortIf(char *interface, struct eth_port_if *pPortIf);
extern int getEthLink(char *interface, struct eth_link_if *pLinkIf);
extern int getEthStats(char *interface, struct user_net_device_stats *pStats);
extern int cwmpSettingChange(int mibId);


#define VENDOR_NAME "EXT_" 
#define CREATE_NAME(node) "X_"VENDOR_NAME#node

int getDev(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int objIFStack(char *name, struct CWMP_LEAF *e, int type, void *data);
int getIFStackEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data);

#define CONFIG_SET(key, val) if ( mib_set(key, val)==0)  return ERR_9002
#define CONFIG_GET(key, ret) if ( mib_get(key, ret)==0)  return ERR_9002

#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;
#define CHECK_PARAM_STR(str, min, max)  do { \
	int tmp; \
	if (!str) return ERR_9007; \
	tmp=strlen(str); \
	if ((tmp < min) || (tmp > max)) return ERR_9007; \
}	while (0)


#ifdef __cplusplus
}
#endif

#endif /*_TR181_DEVICE_H_*/
