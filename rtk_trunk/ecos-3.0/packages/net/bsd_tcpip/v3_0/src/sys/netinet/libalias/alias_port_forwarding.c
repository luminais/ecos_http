/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : PORT FORWARDING SUPPORT
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/
#define  KLD_ENABLED
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
//#include <netinet/fastpath/rtl_queue.h>
#include <rtl/rtl_queue.h>
#include <sys/param.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "alias_local.h"

#define TCP_FLAG	0x1
#define UDP_FLAG    0x2
#define  RTL_PORT_FORWARDING_TBL_LIST_MAX		20
#define  RTL_PORT_FORWARDING_TBL_ENTRY_MAX		20


/*====================CONE NAT STRUCT DEFINE========================*/

typedef struct _rtl_port_forwarding_entry_t
{
	unsigned long 	ipAddr;
	unsigned short 	portDown;
	unsigned short	portUp;
    #ifdef KLD_ENABLED
    unsigned short 	PrivatefromPort;
	unsigned short	PrivatetoPort;
    #endif
	unsigned short	protocol;
}rtl_port_forwarding_entry_t;

struct Rtl_Port_Forwarding_List_Entry
{
	rtl_port_forwarding_entry_t rtl_port_forwarding_entry;

	CTAILQ_ENTRY(Rtl_Port_Forwarding_List_Entry) rtl_port_forwarding_link;
	CTAILQ_ENTRY(Rtl_Port_Forwarding_List_Entry) tqe_link;
};

struct Rtl_Port_Forwarding_Table
{
	CTAILQ_HEAD(Rtl_port_forwarding_list_entry_head, Rtl_Port_Forwarding_List_Entry) *list;
};

CTAILQ_HEAD(Rtl_port_forwarding_list_inuse_head, Rtl_Port_Forwarding_List_Entry) rtl_port_forwarding_list_inuse;
CTAILQ_HEAD(Rtl_port_forwarding_list_free_head, Rtl_Port_Forwarding_List_Entry) rtl_port_forwarding_list_free;

struct Rtl_Port_Forwarding_Table *table_rtl_port_forwarding;
static int rtl_port_forwarding_table_list_max;

 int rtl_port_forwarding_enabled = 1;

 static struct  Rtl_Port_Forwarding_List_Entry *rtl_lookupRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol);

int rtl_initRtlPortFwdTable(int rtl_port_forwarding_tbl_list_max, int rtl_port_forwarding_tbl_entry_max)
{
	int i;

	table_rtl_port_forwarding = (struct Rtl_Port_Forwarding_Table *)kmalloc(sizeof(struct Rtl_Port_Forwarding_Table), GFP_ATOMIC);
	if (table_rtl_port_forwarding == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_port_forwarding) \n");
		return -1;
	}
	CTAILQ_INIT(&rtl_port_forwarding_list_inuse);
	CTAILQ_INIT(&rtl_port_forwarding_list_free);

	rtl_port_forwarding_table_list_max=rtl_port_forwarding_tbl_list_max;

	table_rtl_port_forwarding->list=(struct Rtl_port_forwarding_list_entry_head *)kmalloc(rtl_port_forwarding_table_list_max*sizeof(struct Rtl_port_forwarding_list_entry_head), GFP_ATOMIC);

	if (table_rtl_port_forwarding->list == NULL) {
		diag_printf("MALLOC Failed! (rtl_port_forwarding list) \n");
		return -1;
	}
	for (i=0; i<rtl_port_forwarding_table_list_max; i++) {
		CTAILQ_INIT(&table_rtl_port_forwarding->list[i]);
	}

	for (i=0; i<rtl_port_forwarding_tbl_entry_max; i++) {
		struct Rtl_Port_Forwarding_List_Entry *entry = (struct Rtl_Port_Forwarding_List_Entry *)kmalloc(sizeof(struct Rtl_Port_Forwarding_List_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			diag_printf("MALLOC Failed! (Rtl_Port_Forwarding_List_Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&rtl_port_forwarding_list_free, entry, tqe_link);
	}

	return 0;
}

static inline unsigned int Hash_Rtl_Port_Fwd_Entry(unsigned long ipAddr)
{
	register unsigned int hash;

	hash = ipAddr;
	
	return ((rtl_port_forwarding_table_list_max-1) & hash);
}

#if defined(KLD_ENABLED)
int rtl_addRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short PrivatefromPort, unsigned short PrivatetoPort, unsigned short protocol)
#else
int rtl_addRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol)
#endif
{
	int s;
	unsigned int hash;
	struct Rtl_Port_Forwarding_List_Entry *entry;

	if(rtl_lookupRtlPortFwdEntry(ipAddr, portDown, portUp, protocol))
		return FAILED;	/*entry already exist*/

	s = splimp();
	hash = Hash_Rtl_Port_Fwd_Entry(ipAddr);
	if (!CTAILQ_EMPTY(&rtl_port_forwarding_list_free)) {
		entry = CTAILQ_FIRST(&rtl_port_forwarding_list_free);
		memset(&(entry->rtl_port_forwarding_entry), 0, sizeof(rtl_port_forwarding_entry_t));
		entry->rtl_port_forwarding_entry.ipAddr	=ipAddr;
		entry->rtl_port_forwarding_entry.portDown= portDown;
		entry->rtl_port_forwarding_entry.portUp= portUp;        
        #if defined(KLD_ENABLED)
        entry->rtl_port_forwarding_entry.PrivatefromPort= PrivatefromPort;
		entry->rtl_port_forwarding_entry.PrivatetoPort= PrivatetoPort; 
        #endif
		entry->rtl_port_forwarding_entry.protocol= protocol;
		CTAILQ_REMOVE(&rtl_port_forwarding_list_free, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&rtl_port_forwarding_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&table_rtl_port_forwarding->list[hash], entry, rtl_port_forwarding_link);
		splx(s);
		return SUCCESS;
	}
	splx(s);
	return FAILED;
}

static struct  Rtl_Port_Forwarding_List_Entry *rtl_lookupRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol)
{
	int s;
	unsigned int hash;
	struct Rtl_Port_Forwarding_List_Entry *entry;

	s = splimp();
	hash = Hash_Rtl_Port_Fwd_Entry(ipAddr);
	CTAILQ_FOREACH(entry, &table_rtl_port_forwarding->list[hash], rtl_port_forwarding_link) {
	   if ((entry->rtl_port_forwarding_entry.ipAddr==ipAddr) && (entry->rtl_port_forwarding_entry.portDown==portDown) &&
		(entry->rtl_port_forwarding_entry.portUp==portUp) && (entry->rtl_port_forwarding_entry.protocol==protocol))
		{
			splx(s);
			return entry;
		}
	}
	
	splx(s);
	return NULL;
}

unsigned long rtl_getIntIpByPortFromPortFwdEntry(unsigned short port, unsigned short protocol)
{
	int s;
	int proto_flag = 0;
	struct Rtl_Port_Forwarding_List_Entry *entry;

	if(protocol == IPPROTO_TCP)
		proto_flag |= TCP_FLAG;
	else if(protocol == IPPROTO_UDP)
		proto_flag |= UDP_FLAG;
		
	s = splimp();
	CTAILQ_FOREACH(entry, &rtl_port_forwarding_list_inuse, tqe_link) {
		if ((entry->rtl_port_forwarding_entry.portDown<=ntohs(port)) && (entry->rtl_port_forwarding_entry.portUp>=ntohs(port)) &&
			(entry->rtl_port_forwarding_entry.protocol&proto_flag)){
			splx(s);
			return entry->rtl_port_forwarding_entry.ipAddr;
		}
	}
	splx(s);
	return 0;
}

#if defined(KLD_ENABLED)
int rtl_getIntIpAndPortFromPortFwdEntry(unsigned short mPort, unsigned short protocol, unsigned short *intPort, unsigned long *intIp)
{
	int s;
	int proto_flag = 0;
	struct Rtl_Port_Forwarding_List_Entry *entry;
    #if 0
	if(protocol == IPPROTO_TCP)
		proto_flag |= TCP_FLAG;
	else if(protocol == IPPROTO_UDP)
		proto_flag |= UDP_FLAG;
	#endif	
	s = splimp();
	CTAILQ_FOREACH(entry, &rtl_port_forwarding_list_inuse, tqe_link) {

		if ((entry->rtl_port_forwarding_entry.portDown <= ntohs(mPort)) && 
		    (entry->rtl_port_forwarding_entry.portUp>=ntohs(mPort)) && 
		    ((entry->rtl_port_forwarding_entry.protocol==protocol)||
		    (entry->rtl_port_forwarding_entry.protocol==255))){
			//protocol=255 means (tcp and udp)
			*intPort = htons((entry->rtl_port_forwarding_entry.PrivatefromPort) + ntohs(mPort) - (entry->rtl_port_forwarding_entry.portDown));
			*intIp = entry->rtl_port_forwarding_entry.ipAddr;
			splx(s);
			return SUCCESS;
		}
	}
	splx(s);
	return FAILED;
}
#endif

int rtl_delRtlPortFwdEntry(unsigned long ipAddr, unsigned short portDown, unsigned short portUp, unsigned short protocol)
{
	int s;
	unsigned int hash;
	struct Rtl_Port_Forwarding_List_Entry *entry = NULL;

	hash = Hash_Rtl_Port_Fwd_Entry(ipAddr);
	if(entry=rtl_lookupRtlPortFwdEntry(ipAddr, portDown, portUp, protocol)){
		s = splimp();
		CTAILQ_REMOVE(&table_rtl_port_forwarding->list[hash], entry, rtl_port_forwarding_link);
		CTAILQ_REMOVE(&rtl_port_forwarding_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&rtl_port_forwarding_list_free, entry, tqe_link);
		splx(s);
		return SUCCESS;
	}

	return FAILED;
}

int rtl_flushPortFwdEntry(void)
{
	int i, s;
	unsigned int hash;
	struct Rtl_Port_Forwarding_List_Entry *entry;

	s = splimp();
	for(i=0; i<rtl_port_forwarding_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_rtl_port_forwarding->list[i], rtl_port_forwarding_link) {
			CTAILQ_REMOVE(&table_rtl_port_forwarding->list[i], entry, rtl_port_forwarding_link);
			CTAILQ_REMOVE(&rtl_port_forwarding_list_inuse, entry, tqe_link);
			CTAILQ_INSERT_TAIL(&rtl_port_forwarding_list_free, entry, tqe_link);
		}
	}

	splx(s);
	return SUCCESS;
}

int rtl_initRtlPortFwd(void)
{
	return rtl_initRtlPortFwdTable(RTL_PORT_FORWARDING_TBL_LIST_MAX, RTL_PORT_FORWARDING_TBL_ENTRY_MAX);
}

void rtl_showRtlPortFwdEntry(void)
{
	struct Rtl_Port_Forwarding_List_Entry *entry;

	#if defined(KLD_ENABLED)
	CTAILQ_FOREACH(entry, &rtl_port_forwarding_list_inuse, tqe_link) {
		diag_printf("~RtlPortFwd: Protocol is %d ipAddr=0x%08X public from port=%-5u public to port=%-5u \
            private from port=%-5u private to port=%-5u\n",
			entry->rtl_port_forwarding_entry.protocol, entry->rtl_port_forwarding_entry.ipAddr, 
			entry->rtl_port_forwarding_entry.portDown, entry->rtl_port_forwarding_entry.portUp,
			entry->rtl_port_forwarding_entry.PrivatefromPort, entry->rtl_port_forwarding_entry.PrivatetoPort);
	}
	#else
	CTAILQ_FOREACH(entry, &rtl_port_forwarding_list_inuse, tqe_link) {
		diag_printf("~RtlPortFwd: Protocol is %d ipAddr=0x%08X portDown=%-5u portUp=%-5u\n",
			entry->rtl_port_forwarding_entry.protocol, entry->rtl_port_forwarding_entry.ipAddr, 
			entry->rtl_port_forwarding_entry.portDown, entry->rtl_port_forwarding_entry.portUp);
	}
	#endif
}

