/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : TRIGGER PORT SUPPORT
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



#define TCP_FLAG	0x1
#define UDP_FLAG    0x2
#define  RTL_TRIGGER_PORT_TBL_LIST_MAX		2
#define  RTL_TRIGGER_PORT_TBL_ENTRY_MAX	32
#define  RTL_TRIGGER_PORT_RULE_MAX			128
#define  RTL_TRIGGER_PORT_ENTRY_CHECK		30*hz

#ifdef DEBUG
#undef  pr_debug
#define pr_debug(fmt, args...) printk("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args)
#endif

/*====================TRIGGER PORT STRUCT DEFINE========================*/

enum ipt_trigger_type
{
	IPT_TRIGGER_DNAT = 1,
	IPT_TRIGGER_IN = 2,
	IPT_TRIGGER_OUT = 3
};

struct ipt_trigger_ports {
	unsigned short mport[2];	/* Trigger port range */
	unsigned short rport[2];	/* Related port range */
};

struct ipt_trigger_info {
	unsigned short valid;
	unsigned short rule_no;
	unsigned short mproto;	/* Trigger protocol */
	unsigned short rproto;	/* Related protocol */
	struct ipt_trigger_ports ports;
};

struct ipt_trigger {
    unsigned long last_used;
    unsigned int srcip;		/* Outgoing source address */
    unsigned int dstip;		/* Outgoing destination address */
    unsigned short mproto;		/* Trigger protocol */
    unsigned short rproto;		/* Related protocol */
    struct ipt_trigger_ports ports;	/* Trigger and related ports */
    unsigned char reply;			/* Confirm a reply connection */
};

struct Rtl_Trigger_Port_List_Entry
{
	struct ipt_trigger rtl_trigger_port_entry;

	CTAILQ_ENTRY(Rtl_Trigger_Port_List_Entry) rtl_trigger_port_link;
	CTAILQ_ENTRY(Rtl_Trigger_Port_List_Entry) tqe_link;
};

struct Rtl_Trigger_Port_Table
{
	CTAILQ_HEAD(Rtl_trigger_port_list_entry_head, Rtl_Trigger_Port_List_Entry) *list;
};

CTAILQ_HEAD(Rtl_trigger_port_list_inuse_head, Rtl_Trigger_Port_List_Entry) rtl_trigger_port_list_inuse;
CTAILQ_HEAD(Rtl_trigger_port_list_free_head, Rtl_Trigger_Port_List_Entry) rtl_trigger_port_list_free;

struct Rtl_Trigger_Port_Table *table_rtl_trigger_port;
static int rtl_trigger_port_table_list_max;
int rtl_trigger_port_enabled = 0;
struct callout rtl_trigger_port_entry_timer;
struct ipt_trigger_info rtl_trigger_rule[RTL_TRIGGER_PORT_RULE_MAX];
static unsigned int trigger_ip_timeout = 600;	/* 600s */

extern int rtl_setTriggerPortIpfw(unsigned short rproto, unsigned short rportMin, unsigned short rportMax);
extern int rtl_delTriggerPortIpfw(unsigned short rproto, unsigned short rportMin, unsigned short rportMax);
extern void rtl_flushTriggerPortIpfw(void);
static struct  Rtl_Trigger_Port_List_Entry *rtl_triggerPortEntryMatched(struct alias_link *link, struct ipt_trigger_info *rule);
static void rtl_TriggerPortTimerHandle(void);
static int rtl_triggerPortEntryExpired(void);
static void rtl_setTriggerPortTimeout(char *timeout);


/*-----------------------Trigger port rule add/del API define start--------------------*/
static int rtl_setTriggerPortRule(struct ipt_trigger_info *rule)
{
	int i;

	if (rtl_trigger_port_enabled==0)
		return FAILED;

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if (rtl_trigger_rule[i].valid == 1) {
			if ((rtl_trigger_rule[i].mproto==rule->mproto)&&((rtl_trigger_rule[i].rproto==rule->rproto))&&
		    	     (rtl_trigger_rule[i].ports.mport[0]==rule->ports.mport[0])&&(rtl_trigger_rule[i].ports.mport[1]==rule->ports.mport[1])&&
		    	     (rtl_trigger_rule[i].ports.rport[0]==rule->ports.rport[0])&&(rtl_trigger_rule[i].ports.rport[1]==rule->ports.rport[1]))
			    	return FAILED;	/*entry already exist*/
		}
	}

	if (rtl_trigger_rule[rule->rule_no].valid == 1) {
		diag_printf("Error: rule_no[%d]  has already been used by other rule\n", rule->rule_no);				
		return FAILED;
	}

	memset(&rtl_trigger_rule[rule->rule_no], 0, sizeof(struct ipt_trigger_info));
	rtl_trigger_rule[rule->rule_no].valid 			= 1;
	rtl_trigger_rule[rule->rule_no].rule_no			= rule->rule_no;
	rtl_trigger_rule[rule->rule_no].mproto 			= rule->mproto;
	rtl_trigger_rule[rule->rule_no].ports.mport[0] 	= rule->ports.mport[0];
	rtl_trigger_rule[rule->rule_no].ports.mport[1]	= rule->ports.mport[1];
	rtl_trigger_rule[rule->rule_no].rproto			= rule->rproto;
	rtl_trigger_rule[rule->rule_no].ports.rport[0]		= rule->ports.rport[0];
	rtl_trigger_rule[rule->rule_no].ports.rport[1]		= rule->ports.rport[1];
	rtl_setTriggerPortIpfw(rtl_trigger_rule[rule->rule_no].rproto, rtl_trigger_rule[rule->rule_no].ports.rport[0],
						   rtl_trigger_rule[rule->rule_no].ports.rport[1]);
	return SUCCESS;
}

int rtl_addTriggerPortRule(unsigned short rule_no, unsigned short mproto, unsigned short rproto,
									     unsigned short mport[2], unsigned short rport[2])
{
	struct ipt_trigger_info rule;

	if (rule_no >= RTL_TRIGGER_PORT_RULE_MAX) {
		diag_printf("Error: Rule no %d is bigger than Max threshold %d\n", rule_no, RTL_TRIGGER_PORT_RULE_MAX);
		return FAILED;
	}
	
	rule.rule_no	= rule_no;
	rule.mproto 	= mproto;
	rule.rproto	= rproto;
	rule.ports.mport[0] = mport[0];
	rule.ports.mport[1] = mport[1];
	rule.ports.rport[0] 	 = rport[0];
	rule.ports.rport[1] 	 = rport[1];

	return rtl_setTriggerPortRule(&rule);
}
	
static int rtl_delTriggerRule(struct ipt_trigger_info *rule)
{
	int i;

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if ((rtl_trigger_rule[i].valid==1)&&(rtl_trigger_rule[i].mproto==rule->mproto)&&((rtl_trigger_rule[i].rproto==rule->rproto))&&
		    (rtl_trigger_rule[i].ports.mport[0]==rule->ports.mport[0])&&(rtl_trigger_rule[i].ports.mport[1]==rule->ports.mport[1])&&
	           (rtl_trigger_rule[i].ports.rport[0]==rule->ports.rport[0])&&(rtl_trigger_rule[i].ports.rport[1]==rule->ports.rport[1])) {
			   rtl_trigger_rule[i].valid = 0;
			   return SUCCESS;
		}
	}

	return FAILED;
}

int rtl_setTriggerPort(unsigned int argc, unsigned char *argv[])
{
	int arg;
	int ret = SUCCESS;
	char tmpBuff[32];
	struct ipt_trigger_info trigger_tmp;

	if (!strcmp(argv[1], "timeout"))
		rtl_setTriggerPortTimeout(argv[2]);
	#if 0
	else if (!strcmp(argv[1], "rule")) {
		 memset(&trigger_tmp, 0, sizeof(struct ipt_trigger_info));
		 
		 arg = 2;
		 while (arg<argc)
		 {
			if ((!strcmp(argv[arg], "ruleNo"))) {
				if (argv[++arg]) {
					strcpy(tmpBuff, argv[arg]);
					sscanf(tmpBuff, "%hu", &trigger_tmp.rule_no);
					if (trigger_tmp.rule_no >= RTL_TRIGGER_PORT_RULE_MAX) {
						diag_printf("Error: Rule no %d is bigger than Max threshold %d\n", trigger_tmp.rule_no, RTL_TRIGGER_PORT_RULE_MAX);
						ret = FAILED;
						goto OUT;
					}
					arg++;
				} else {
					ret = FAILED;
					goto OUT;
				}
			} else if ((!strcmp(argv[arg], "mproto"))) {
				if (argv[++arg]) {
					strcpy(tmpBuff, argv[arg]);
					if (!strcmp(tmpBuff, "tcp")) {
						trigger_tmp.mproto = IPPROTO_TCP;
					} else if (!strcmp(tmpBuff, "udp")) {
						trigger_tmp.mproto = IPPROTO_UDP;
					} else {
						diag_printf("Error: mproto %s is not suppported for trigger port feature\n", tmpBuff);
						ret = FAILED;
						goto OUT;
					}
					arg++;
				} else {
					ret = FAILED;
					goto OUT;
				}
			} else if ((!strcmp(argv[arg], "mports"))) {
				if (argv[++arg]) {
					strcpy(tmpBuff, argv[arg]);
					sscanf(tmpBuff, "%hu-%hu", &trigger_tmp.ports.mport[0], &trigger_tmp.ports.mport[1]);
					arg++;
				} else {
					ret = FAILED;
					goto OUT;
				}
			} else if ((!strcmp(argv[arg], "rproto"))) {
				if (argv[++arg]) {
					strcpy(tmpBuff, argv[arg]);
					if (!strcmp(tmpBuff, "tcp")) {
						trigger_tmp.rproto = IPPROTO_TCP;
					} else if (!strcmp(tmpBuff, "udp")) {
						trigger_tmp.rproto = IPPROTO_UDP;
					} else {
						diag_printf("Error: rproto %s is not suppported for trigger port feature\n", tmpBuff);
						ret = FAILED;
						goto OUT;
					}
					arg++;
				} else {
					ret = FAILED;
					goto OUT;
				}
			} else if ((!strcmp(argv[arg], "rports"))) {
				if (argv[++arg]) {
					strcpy(tmpBuff, argv[arg]);
					sscanf(tmpBuff, "%hu-%hu", &trigger_tmp.ports.rport[0], &trigger_tmp.ports.rport[1]);
					arg++;
				} else {
					ret = FAILED;
					goto OUT;
				}
			}
		 }
		
		ret = rtl_setTriggerPortRule(&trigger_tmp);
	} 
	
OUT:
	#endif
	return ret;
}


void rtl_flushTriggerPortRule(void)
{
	int i;

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++) {
		rtl_trigger_rule[i].valid = 0;
	}
	
	rtl_flushTriggerPortIpfw();
}

static void rtl_showTriggerPortRule(void)
{
	int i;

	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if (rtl_trigger_rule[i].valid==1) {
			diag_printf("RuleNo[%d]: mproto is %d, mports is %d-%d, rproto is %d, rports is %d-%d\n", rtl_trigger_rule[i].rule_no,
						rtl_trigger_rule[i].mproto, rtl_trigger_rule[i].ports.mport[0], rtl_trigger_rule[i].ports.mport[1],
						rtl_trigger_rule[i].rproto, rtl_trigger_rule[i].ports.rport[0], rtl_trigger_rule[i].ports.rport[1]);
		}	
	}
}
	
	
/*-----------------------Trigger port rule add/del API define end--------------------*/


/*-----------------------Trigger port timer API define start--------------------*/
static void rtl_initTriggerPortTimer(void)
{
	callout_init(&rtl_trigger_port_entry_timer);
	callout_reset(&rtl_trigger_port_entry_timer, RTL_TRIGGER_PORT_ENTRY_CHECK, rtl_TriggerPortTimerHandle, 0);
}

static void rtl_TriggerPortTimerHandle(void)
{
	rtl_triggerPortEntryExpired();
	callout_reset(&rtl_trigger_port_entry_timer, RTL_TRIGGER_PORT_ENTRY_CHECK, rtl_TriggerPortTimerHandle, 0);
}

static void rtl_setTriggerPortTimeout(char *timeout)
{
	unsigned long expire_value = 0;
	char tmp[4];

	if (timeout) {
		strncpy(tmp, timeout, 4);
		sscanf(tmp, "%d", &expire_value);
		trigger_ip_timeout = expire_value;
	}
}

/*-----------------------Trigger port timer API define end--------------------*/



/*-----------------------Trigger port entry add/del/flush API define start--------------------*/

int rtl_initTriggerPortTable(int rtl_trigger_port_tbl_list_max, int rtl_trigger_port_tbl_entry_max)
{
	int i;

	table_rtl_trigger_port = (struct Rtl_Trigger_Port_Table *)kmalloc(sizeof(struct Rtl_Trigger_Port_Table), GFP_ATOMIC);
	if (table_rtl_trigger_port == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_trigger_port) \n");
		return FAILED;
	}
	CTAILQ_INIT(&rtl_trigger_port_list_inuse);
	CTAILQ_INIT(&rtl_trigger_port_list_free);

	rtl_trigger_port_table_list_max=rtl_trigger_port_tbl_list_max;
	table_rtl_trigger_port->list=(struct Rtl_trigger_port_list_entry_head *)kmalloc(rtl_trigger_port_table_list_max*sizeof(struct Rtl_trigger_port_list_entry_head), GFP_ATOMIC);

	if (table_rtl_trigger_port->list == NULL) {
		diag_printf("MALLOC Failed! (rtl_port_forwarding list) \n");
		return FAILED;
	}
	for (i=0; i<rtl_trigger_port_table_list_max; i++) {
		CTAILQ_INIT(&table_rtl_trigger_port->list[i]);
	}

	for (i=0; i<rtl_trigger_port_tbl_entry_max; i++) {
		struct Rtl_Trigger_Port_List_Entry *entry = (struct Rtl_Trigger_Port_List_Entry *)kmalloc(sizeof(struct Rtl_Trigger_Port_List_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			diag_printf("MALLOC Failed! (Rtl_Trigger_Port_List_Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&rtl_trigger_port_list_free, entry, tqe_link);
	}

	return SUCCESS;
}

/*hash by related protocol*/
static inline unsigned int Hash_Rtl_Trigger_Port_Entry(unsigned short rProto)
{
	register unsigned int hash = rProto;
	
	return ((rtl_trigger_port_table_list_max-1)&hash);
}

int rtl_triggerOut(struct alias_link *link)
{
	int s, i, mProto;
	unsigned int hash;
	struct Rtl_Trigger_Port_List_Entry *entry;
	struct ipt_trigger_info *rule;

	if ((link->link_type!=IPPROTO_TCP) && (link->link_type!=IPPROTO_UDP))
		return FAILED;	/*Only process tcp or udp*/

	s = splimp();
	/*Check if this link match any trigger port rule*/
	for (i=0; i<RTL_TRIGGER_PORT_RULE_MAX; i++)
	{
		if ((rtl_trigger_rule[i].valid==1)&&(rtl_trigger_rule[i].mproto==link->link_type)&&
		     ((link->dst_port)>=rtl_trigger_rule[i].ports.mport[0])&&
		     ((link->dst_port)<=rtl_trigger_rule[i].ports.mport[1])) {
		     		rule = &rtl_trigger_rule[i];
				if(entry=rtl_triggerPortEntryMatched(link, rule)) {
					entry->rtl_trigger_port_entry.last_used = (unsigned long)jiffies;

					 /* In order to allow multiple hosts use the same port range, we update
			           the 'saddr' after previous trigger has a reply connection. */
			       		if (entry->rtl_trigger_port_entry.reply)
			           		 entry->rtl_trigger_port_entry.srcip = link->src_addr.s_addr;
					continue;	/*entry already exist*/
				}

				hash = Hash_Rtl_Trigger_Port_Entry(rule->rproto);
				if(!CTAILQ_EMPTY(&rtl_trigger_port_list_free)) {
					entry = CTAILQ_FIRST(&rtl_trigger_port_list_free);
					memset(&(entry->rtl_trigger_port_entry), 0, sizeof(struct ipt_trigger));
					entry->rtl_trigger_port_entry.last_used	= (unsigned long)jiffies;
					entry->rtl_trigger_port_entry.srcip 		= link->src_addr.s_addr;
					entry->rtl_trigger_port_entry.mproto		= rule->mproto;
					entry->rtl_trigger_port_entry.rproto		= rule->rproto;
					memcpy(&entry->rtl_trigger_port_entry.ports, &rule->ports, sizeof(struct ipt_trigger_ports));
					
					CTAILQ_REMOVE(&rtl_trigger_port_list_free, entry, tqe_link);
					CTAILQ_INSERT_TAIL(&rtl_trigger_port_list_inuse, entry, tqe_link);
					CTAILQ_INSERT_TAIL(&table_rtl_trigger_port->list[hash], entry, rtl_trigger_port_link);
				}
		}
	}
	splx(s);
	return SUCCESS;
}

/*wam->lan pkt, proto is pkt ip hdr info*/
unsigned int rtl_triggerDnat(unsigned short dport, unsigned short proto)
{
	int s;
	struct Rtl_Trigger_Port_List_Entry *entry;

	s = splimp();
	CTAILQ_FOREACH(entry, &rtl_trigger_port_list_inuse, tqe_link) {
		if ((entry->rtl_trigger_port_entry.rproto==proto)&&(entry->rtl_trigger_port_entry.ports.rport[0]<=dport)&&
		     (entry->rtl_trigger_port_entry.ports.rport[1]>=dport)){
			 entry->rtl_trigger_port_entry.reply 		= 1;
			 entry->rtl_trigger_port_entry.last_used 	= (unsigned long)jiffies;
			 splx(s);
			 return entry->rtl_trigger_port_entry.srcip;
		}
	}
	splx(s);
	return 0;
}

static struct  Rtl_Trigger_Port_List_Entry *rtl_triggerPortEntryMatched(struct alias_link *link, struct ipt_trigger_info *rule)
{
	int s;
	unsigned int hash;
	struct Rtl_Trigger_Port_List_Entry *entry;

	s = splimp();
	hash = Hash_Rtl_Trigger_Port_Entry(rule->rproto);
	CTAILQ_FOREACH(entry, &table_rtl_trigger_port->list[hash], rtl_trigger_port_link) {
	   if ((entry->rtl_trigger_port_entry.mproto==link->link_type)&&(entry->rtl_trigger_port_entry.ports.mport[0]<=link->dst_port)&&
		(entry->rtl_trigger_port_entry.ports.mport[1]>=link->dst_port)&&(entry->rtl_trigger_port_entry.rproto==rule->rproto)&&
		(entry->rtl_trigger_port_entry.ports.rport[0]==rule->ports.rport[0])&&(entry->rtl_trigger_port_entry.ports.rport[1]==rule->ports.rport[1])) {
			splx(s);
			return entry;
		}
	}
	
	splx(s);
	return NULL;
}

static int rtl_triggerPortEntryExpired(void)
{
	int i, s;
	unsigned int hash;
	struct Rtl_Trigger_Port_List_Entry *entry;
	unsigned long now = (unsigned long)jiffies;
	
	s = splimp();
	for(i=0; i<rtl_trigger_port_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_rtl_trigger_port->list[i], rtl_trigger_port_link) {
			if (((entry->rtl_trigger_port_entry.last_used+trigger_ip_timeout*HZ) < now))
			{
				CTAILQ_REMOVE(&table_rtl_trigger_port->list[hash], entry, rtl_trigger_port_link);
				CTAILQ_REMOVE(&rtl_trigger_port_list_inuse, entry, tqe_link);
				CTAILQ_INSERT_TAIL(&rtl_trigger_port_list_free, entry, tqe_link);
			}
		}
	}

	splx(s);
	return SUCCESS;
}

static int rtl_flushTriggerPortEntry(void)
{
	int i, s;
	unsigned int hash;
	struct Rtl_Trigger_Port_List_Entry *entry;

	s = splimp();
	for(i=0; i<rtl_trigger_port_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_rtl_trigger_port->list[i], rtl_trigger_port_link) {
			CTAILQ_REMOVE(&table_rtl_trigger_port->list[i], entry, rtl_trigger_port_link);
			CTAILQ_REMOVE(&rtl_trigger_port_list_inuse, entry, tqe_link);
			CTAILQ_INSERT_TAIL(&rtl_trigger_port_list_free, entry, tqe_link);
		}
	}

	splx(s);
	return SUCCESS;
}

static void rtl_showTriggerPortEntry(void)
{
	struct Rtl_Trigger_Port_List_Entry *entry;

	diag_printf("Trigger Port Entry expire time is %d\n", trigger_ip_timeout);
	
	CTAILQ_FOREACH(entry, &rtl_trigger_port_list_inuse, tqe_link) {
		diag_printf("~RtlTriggerPort: srcip=0x%08X mProto is %d rProto is %d mPortDown=%-5u mPortUp=%-5u rPortDown=%-5u rPortUp=%-5u\n",
			entry->rtl_trigger_port_entry.srcip, entry->rtl_trigger_port_entry.mproto, entry->rtl_trigger_port_entry.rproto,
			entry->rtl_trigger_port_entry.ports.mport[0], entry->rtl_trigger_port_entry.ports.mport[1],
			entry->rtl_trigger_port_entry.ports.rport[0], entry->rtl_trigger_port_entry.ports.rport[1]);
	}
}

/*-----------------------Trigger port entry add/del/flush API define end--------------------*/

void rtl_flushTriggerPort(char *parm)
{
	if (parm) {
		if (!strcmp(parm,"entry"))
			rtl_flushTriggerPortEntry();
		else if (!strcmp(parm,"rule"))
			rtl_flushTriggerPortRule();
	}
}

void rtl_showTriggerPort(char *parm)
{
	if (parm) {
		if (!strcmp(parm,"entry"))
			rtl_showTriggerPortEntry();
		else if (!strcmp(parm,"rule"))
			rtl_showTriggerPortRule();
	}
}

int rtl_delTriggerRuleByNum(char *parm)
{
	int ruleNo;

	if (parm) {
		sscanf(parm, "%d", &ruleNo);
		if (ruleNo >= RTL_TRIGGER_PORT_RULE_MAX) {
			diag_printf("The ruleNo[%d] is out of range!\n", ruleNo);
			return FAILED;
		}

		if (rtl_trigger_rule[ruleNo].valid==1)
			rtl_trigger_rule[ruleNo].valid = 0;
		else
			diag_printf("The ruleNo[%d] is not used yet!\n", ruleNo);

		rtl_delTriggerPortIpfw(rtl_trigger_rule[ruleNo].rproto, rtl_trigger_rule[ruleNo].ports.rport[0], 
							   rtl_trigger_rule[ruleNo].ports.rport[1]);
		return SUCCESS;
	}

	return FAILED;
}

void rtl_enableTriggerPort(int value)
{
	rtl_trigger_port_enabled = value;
}

int rtl_initTriggerPort(void)
{
	memset(rtl_trigger_rule, 0, RTL_TRIGGER_PORT_RULE_MAX*sizeof(struct ipt_trigger_info));
	rtl_initTriggerPortTimer();
	
	return rtl_initTriggerPortTable(RTL_TRIGGER_PORT_TBL_LIST_MAX, RTL_TRIGGER_PORT_TBL_ENTRY_MAX);
}



