#ifndef _PRMT_WANCONDEVICE_H_
#define _PRMT_WANCONDEVICE_H_

#include "prmt_igd.h"

#ifdef __cplusplus
extern "C" {
#endif


extern struct CWMP_LEAF tWANCONSTATSLeaf[];
extern struct CWMP_LEAF tPORTMAPENTITYLeaf[];
extern struct CWMP_LINKNODE tWANPORTMAPObject[];
extern struct CWMP_LEAF tWANPPPCONENTITYLeaf[];
extern struct CWMP_NODE tWANPPPCONENTITYObject[];
extern struct CWMP_LINKNODE tWANPPPCONObject[];
extern struct CWMP_LEAF tWANIPCONENTITYLeaf[];
extern struct CWMP_NODE tWANIPCONENTITYObject[];
extern struct CWMP_LINKNODE tWANIPCONObject[];
extern struct CWMP_LEAF tDSLLNKCONFLeaf[];
extern struct CWMP_LEAF tWANCONDEVENTITYLeaf[];
extern struct CWMP_NODE tWANCONDEVENTITYObject[];
extern struct CWMP_LINKNODE tWANCONDEVObject[];
extern struct CWMP_LEAF tCONSERENTITYLeaf[];
extern struct CWMP_LINKNODE tCONSERVICEObject[];
extern struct CWMP_LEAF tWANDSLCNTMNGLeaf[];
extern struct CWMP_NODE tWANDSLCNTMNGObject[];

//by cairui
extern int isConnectPPP(void);

int getPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setPORMAPTENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANPORTMAPPING(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANCONSTATS(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANPPPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANPPPConn(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setWANIPCONENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data);
int objWANIPConn(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int setDSLLNKCONF(char *name, struct CWMP_LEAF *entity, int type, void *data);
int getWANCONDEVENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objConDev(char *name, struct CWMP_LEAF *entity, int type, void *data);

int getCONSERENTITY(char *name, struct CWMP_LEAF *entity, int *type, void **data);

int getWANDSLCNTMNG(char *name, struct CWMP_LEAF *entity, int *type, void **data);
int objConService(char *name, struct CWMP_LEAF *entity, int type, void *data);

/*utilities*/
int getDefaultRouteIfaceName( char *name ); 
int setDefaultRouteIfaceName( char *name );
int transfer2IfName( char *name, char *ifname );
unsigned int transfer2IfIndex( char *name );
int transfer2PathName( unsigned int ifindex, char *name );
int transfer2PathNamefromItf( char *ifname, char *pathname );

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
extern int gStartReset;
void cwmpStartReset();
#endif
/*ping_zhang:20081217 END*/
#ifdef __cplusplus
}
#endif


#endif /*_PRMT_WANCONDEVICE_H_*/
