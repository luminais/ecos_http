/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : CONE NAT SUPPORT
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/

#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
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

//#include <netinet/fastpath/rtl_alias.h>
#include <rtl/rtl_alias.h>

#define  RTL_CONE_NAT_TBL_ENTRY_EXPIRE		60
#define  RTL_CONE_NAT_TBL_ENTRY_CHECK		5*hz
#define  RTL_CONE_NAT_TBL_LIST_MAX			512
#define  RTL_CONE_NAT_TBL_ENTRY_MAX		1024


/*====================CONE NAT STRUCT DEFINE========================*/

struct Rtl_Cone_Nat_List_Entry
{
	rtl_cone_nat_entry_t rtl_cone_nat_entry;

	CTAILQ_ENTRY(Rtl_Cone_Nat_List_Entry) rtl_cone_nat_link;
	CTAILQ_ENTRY(Rtl_Cone_Nat_List_Entry) tqe_link;
};

struct Rtl_Cone_Nat_Table
{
	CTAILQ_HEAD(Rtl_cone_nat_list_entry_head, Rtl_Cone_Nat_List_Entry) *list;
};

CTAILQ_HEAD(Rtl_cone_nat_list_inuse_head, Rtl_Cone_Nat_List_Entry) rtl_cone_nat_list_inuse;
CTAILQ_HEAD(Rtl_cone_nat_list_free_head, Rtl_Cone_Nat_List_Entry) rtl_cone_nat_list_free;

struct Rtl_Cone_Nat_Table *table_rtl_cone_nat;
static int rtl_cone_nat_table_list_max;

int rtl_cone_nat_mode = SYMMETRIC_NAT;

/*====================CONE NAT TIMER DEFINE======================*/

int rtl_flushExpiredConeNatEntry();
int rtl_checkRtlConeNatEntryExist(struct alias_link *link);
struct callout rtl_cone_nat_tbl_timer;

void rtl_ConeNatTimerHandle(void)
{
	rtl_flushExpiredConeNatEntry();
	callout_reset(&rtl_cone_nat_tbl_timer, RTL_CONE_NAT_TBL_ENTRY_CHECK, rtl_ConeNatTimerHandle, 0);
}

static void rtl_initRtlConeNatTimer(void)
{
	callout_init(&rtl_cone_nat_tbl_timer);
	callout_reset(&rtl_cone_nat_tbl_timer, RTL_CONE_NAT_TBL_ENTRY_CHECK, rtl_ConeNatTimerHandle, 0);
}


/*====================CONE NAT API DEFINE======================*/

int rtl_initRtlConeNatTable(int rtl_cone_nat_tbl_list_max, int rtl_cone_nat_tbl_entry_max)
{
	int i;

	table_rtl_cone_nat = (struct Rtl_Cone_Nat_Table *)kmalloc(sizeof(struct Rtl_Cone_Nat_Table), GFP_ATOMIC);
	if (table_rtl_cone_nat == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_cone_nat) \n");
		return -1;
	}
	CTAILQ_INIT(&rtl_cone_nat_list_inuse);
	CTAILQ_INIT(&rtl_cone_nat_list_free);

	rtl_cone_nat_table_list_max=rtl_cone_nat_tbl_list_max;
	table_rtl_cone_nat->list=(struct Rtl_cone_nat_list_entry_head *)kmalloc(rtl_cone_nat_table_list_max*sizeof(struct Rtl_cone_nat_list_entry_head), GFP_ATOMIC);

	if (table_rtl_cone_nat->list == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_cone_nat list) \n");
		return -1;
	}
	for (i=0; i<rtl_cone_nat_table_list_max; i++) {
		CTAILQ_INIT(&table_rtl_cone_nat->list[i]);
	}

	for (i=0; i<rtl_cone_nat_tbl_entry_max; i++) {
		struct Rtl_Cone_Nat_List_Entry *entry = (struct Rtl_Cone_Nat_List_Entry *)kmalloc(sizeof(struct Rtl_Cone_Nat_List_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			diag_printf("MALLOC Failed! (Rtl_Cone_Nat_List_Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&rtl_cone_nat_list_free, entry, tqe_link);
	}

	return 0;
}

static inline unsigned int Hash_Rtl_Cone_Nat_Entry(unsigned long extIp, unsigned short extPort, unsigned short dir, unsigned short protocol)
{
	register unsigned int hash;

	hash = ((extIp>>16)^extIp);
	hash ^= extPort;
	hash ^= dir;
	hash ^= protocol;
	
	return (rtl_cone_nat_table_list_max-1) & (hash ^ (hash >> 12));
}

int rtl_addRtlConeNatEntry(struct alias_link *link)
{
	int s;
	unsigned int hash;
	struct Rtl_Cone_Nat_List_Entry *entry;

	if(rtl_checkRtlConeNatEntryExist(link)==true)
		return FAILED;

	s = splimp();
	hash = Hash_Rtl_Cone_Nat_Entry(link->alias_addr.s_addr, link->alias_port, link->dir, link->link_type);
	if(!CTAILQ_EMPTY(&rtl_cone_nat_list_free)) {
		entry = CTAILQ_FIRST(&rtl_cone_nat_list_free);
		memset(&(entry->rtl_cone_nat_entry), 0, sizeof(rtl_cone_nat_entry_t));
		switch (rtl_cone_nat_mode)
		{
			case FULL_CONE__NAT:
				entry->rtl_cone_nat_entry.remIp 		= NULL;
				entry->rtl_cone_nat_entry.remPort	= NULL;
				break;
			case RESTRICTED_CONE_NAT:
				entry->rtl_cone_nat_entry.remIp 		= link->dst_addr.s_addr;
				entry->rtl_cone_nat_entry.remPort 	= NULL;
				break;
			case PORT_RESTRICTED_CONE_NAT:
				entry->rtl_cone_nat_entry.remIp 		= link->dst_addr.s_addr;
				entry->rtl_cone_nat_entry.remPort 	= link->dst_port;
				break;
			default:
			   	splx(s);
				return FAILED;
		}

		entry->rtl_cone_nat_entry.valid 		= 1;
		entry->rtl_cone_nat_entry.course 	= link->dir;
		entry->rtl_cone_nat_entry.protocol	= link->link_type;
		entry->rtl_cone_nat_entry.intIp		= link->src_addr.s_addr;
		entry->rtl_cone_nat_entry.intPort	= link->src_port;
		entry->rtl_cone_nat_entry.extIp		= link->alias_addr.s_addr;
		entry->rtl_cone_nat_entry.extPort	= link->alias_port;
		entry->rtl_cone_nat_entry.last_used 	= (unsigned long)jiffies;

		CTAILQ_REMOVE(&rtl_cone_nat_list_free, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&rtl_cone_nat_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&table_rtl_cone_nat->list[hash], entry, rtl_cone_nat_link);
		splx(s);
		return SUCCESS;
	}
	splx(s);
	return FAILED;
}

struct  Rtl_Cone_Nat_List_Entry *rtl_lookupRtlConeNatEntry(rtl_cone_nat_entry_t *nat_entry)
{
	int s;
	unsigned int hash;
	unsigned short course;
	struct Rtl_Cone_Nat_List_Entry *entry;

	/*For OUTBOUND link, alg check will be called in alias_in, just INBOUND_DIR*/
	course = (nat_entry->course==INBOUND_DIR)?OUTBOUND_DIR:INBOUND_DIR;
	s = splimp();
	hash = Hash_Rtl_Cone_Nat_Entry(nat_entry->extIp, nat_entry->extPort, course, nat_entry->protocol);
	CTAILQ_FOREACH(entry, &table_rtl_cone_nat->list[hash], rtl_cone_nat_link) {
	   if ((entry->rtl_cone_nat_entry.valid==1) && (entry->rtl_cone_nat_entry.course==course) &&
		(entry->rtl_cone_nat_entry.protocol==nat_entry->protocol) && (entry->rtl_cone_nat_entry.extIp==nat_entry->extIp) &&
		(entry->rtl_cone_nat_entry.extPort==nat_entry->extPort))
		{
			switch (rtl_cone_nat_mode)
			{
				case FULL_CONE__NAT:
					 break;
				case RESTRICTED_CONE_NAT:
					 if(entry->rtl_cone_nat_entry.remIp != nat_entry->remIp)
						continue;
					break;
				case PORT_RESTRICTED_CONE_NAT:
					 if((entry->rtl_cone_nat_entry.remIp != nat_entry->remIp) ||
					 	(entry->rtl_cone_nat_entry.remPort != nat_entry->remPort))
						continue;
					break;
				default:
				   	splx(s);
					return NULL;
			}
			
			entry->rtl_cone_nat_entry.last_used = (unsigned long)jiffies;
			splx(s);
			return entry;
		}
	}
	
	splx(s);
	return NULL;
}

int rtl_getIntIpPortFromConeNatEntry(rtl_cone_nat_entry_t *nat_entry, unsigned long *ip, unsigned short *port)
{
	struct  Rtl_Cone_Nat_List_Entry *entry;

	entry = rtl_lookupRtlConeNatEntry(nat_entry);
	if(entry){
		*ip = entry->rtl_cone_nat_entry.intIp;
		*port = entry->rtl_cone_nat_entry.intPort;
		return SUCCESS;
	}

	return FAILED;
}

int rtl_checkRtlConeNatEntryExist(struct alias_link *link)
{
	int ret = false;
	struct Rtl_Cone_Nat_List_Entry *entry = NULL;
	rtl_cone_nat_entry_t nat_entry;

	memset(&nat_entry, 0, sizeof(rtl_cone_nat_entry_t));
	nat_entry.course 		= INBOUND_DIR;
	nat_entry.protocol 		= link->link_type;
	nat_entry.extIp 		= link->alias_addr.s_addr;
	nat_entry.extPort		= link->alias_port;
	nat_entry.remIp		= link->dst_addr.s_addr;
	nat_entry.remPort		= link->dst_port;
	
	entry = rtl_lookupRtlConeNatEntry(&nat_entry);
	if(entry)
		ret = true;

	return ret;
}

int rtl_flushExpiredConeNatEntry(void)
{
	int i, s;
	unsigned int hash;
	struct Rtl_Cone_Nat_List_Entry *entry;
	unsigned long now = (unsigned long)jiffies;
	s = splimp();
	
	for(i=0; i<rtl_cone_nat_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_rtl_cone_nat->list[i], rtl_cone_nat_link) {
			if ((entry->rtl_cone_nat_entry.valid==1) && ((entry->rtl_cone_nat_entry.last_used+RTL_CONE_NAT_TBL_ENTRY_EXPIRE*HZ) < now))
			{
				entry->rtl_cone_nat_entry.valid = 0;
				CTAILQ_REMOVE(&table_rtl_cone_nat->list[hash], entry, rtl_cone_nat_link);
				CTAILQ_REMOVE(&rtl_cone_nat_list_inuse, entry, tqe_link);
				CTAILQ_INSERT_TAIL(&rtl_cone_nat_list_free, entry, tqe_link);
			}
		}
	}

	splx(s);
	return SUCCESS;
}

int rtl_initRtlConeNat(void)
{
	int ret;
	rtl_initRtlConeNatTimer();

	ret = rtl_initRtlConeNatTable(RTL_CONE_NAT_TBL_LIST_MAX, RTL_CONE_NAT_TBL_ENTRY_MAX);
	return ret;
}

void rtl_showRtlConeNatEntry(void)
{
	struct Rtl_Cone_Nat_List_Entry *entry;

	CTAILQ_FOREACH(entry, &rtl_cone_nat_list_inuse, tqe_link) {
		diag_printf("~RtlConeNat: [mode: %d][dir: %d] [protocol: %d] [valid: %d] int=0x%08X:%-5u ext=0x%08X:%-5u rem=0x%08X:%-5u\n",
			rtl_cone_nat_mode, entry->rtl_cone_nat_entry.course, entry->rtl_cone_nat_entry.protocol, entry->rtl_cone_nat_entry.valid, 
			entry->rtl_cone_nat_entry.intIp, entry->rtl_cone_nat_entry.intPort, entry->rtl_cone_nat_entry.extIp, entry->rtl_cone_nat_entry.extPort,
			entry->rtl_cone_nat_entry.remIp, entry->rtl_cone_nat_entry.remPort);
	}
}


