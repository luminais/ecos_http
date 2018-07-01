/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : Software Vlan process
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/
#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
//#include <netinet/fastpath/rtl_queue.h>
#include <rtl/rtl_queue.h>
#include <netinet/rtl_vlan.h>
#include <sys/param.h>

#if defined(__ECOS)&&!defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
#define  TAG_TABLE_FOR_NATD_TIMEOUT	2
#define  TAG_TABLE_FOR_NATD_CHECK		(hz/2)
struct callout tag_for_natd_tbl_timer;
void rtl_tagForNatdTimeout(void);
#endif
 #if defined(CONFIG_RTL_819X_SWCORE)
extern int rtl_isWanDevDecideByName(unsigned char *name);
 #endif
#define TEST_PACKETS(mac) ((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]==0x00)&&(mac[3]==0x00)&&(mac[4]==0x00)&&(mac[5]==0x1))
static int wanForwardTag = true;
static int wanApOriTag = false;
static rtl_vlan_entry_t *swVlanTbl = NULL;
static unsigned char currVlanDecisionMode = PORT_BASED_VLAN_MODE;
static wan_vlan_tag_mode wanVlanTagMode = KEEP_ORI_VLAN_MODE;

/*Here port is not phy port, juset virtual port!!!*/
static port_mapping_interface portMapInt[MAX_INTERFACE_NUM] = 
{
	{RTL_DRV_LAN0_NETIF_NAME, 0x1, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_LAN1_NETIF_NAME, 0x2, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_LAN2_NETIF_NAME, 0x4, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_LAN3_NETIF_NAME, 0x8, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WAN_NETIF_NAME, 0x10, RTL_WAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P0_NETIF_NAME, 0x20, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P0_VA0_NAME, 0x40, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P0_VA1_NAME, 0x80, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P0_VA2_NAME, 0x100, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P0_VA3_NAME, 0x200, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P1_NETIF_NAME, 0x400, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P1_VA0_NAME, 0x800, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P1_VA1_NAME, 0x1000, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P1_VA2_NAME, 0x2000, RTL_LAN_DEFAULT_PVID, 0, 0},
	{RTL_DRV_WLAN_P1_VA3_NAME, 0x4000, RTL_LAN_DEFAULT_PVID, 0, 0},
	#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
	{RTL_DRV_LAN7_P7_VNETIF_NAME, 0x8000, RTL_WAN_DEFAULT_PVID, 0, 0},
	#endif
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	{RTL_DRV_LAN8_P8_VNETIF_NAME, 0x10000, RTL_WAN_DEFAULT_PVID, 0, 0},
	#endif
};

#define STRIP_TAG(skb)	{	\
	memmove(skb->data+VLAN_HLEN, skb->data, ETH_ALEN*2);	\
	skb_pull(skb, VLAN_HLEN);	\
}

#if defined(__ECOS)&&!defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
struct Tag_For_Natd_List_Entry
{
	unsigned char			valid;
	unsigned short		course;		/* 1:Out-Bonud 2:In-Bound */	
	unsigned short 		protocol;
	unsigned short		id;
	unsigned long			sIp;
	unsigned long			dIp;
	long 				last_used;
	struct vlan_tag 		tag;

	CTAILQ_ENTRY(Tag_For_Natd_List_Entry) tag_for_natd_link;
	CTAILQ_ENTRY(Tag_For_Natd_List_Entry) tqe_link;
};

struct Tag_For_Natd_Table
{
	CTAILQ_HEAD(Tag_for_natd_list_entry_head, Tag_For_Natd_List_Entry) *list;
};

CTAILQ_HEAD(Tag_for_natd_list_inuse_head, Tag_For_Natd_List_Entry) tag_for_natd_list_inuse;
CTAILQ_HEAD(Tag_for_natd_list_free_head, Tag_For_Natd_List_Entry) tag_for_natd_list_free;

struct Tag_For_Natd_Table *table_tag_for_natd;
static int tag_for_natd_table_list_max;
#endif

struct Rtl_Vlan_List_Entry
{
	rtl_vlan_entry_t vlan_entry;

	CTAILQ_ENTRY(Rtl_Vlan_List_Entry) rtl_vlan_link;
	CTAILQ_ENTRY(Rtl_Vlan_List_Entry) tqe_link;
};

struct Rtl_Vlan_Table
{
	CTAILQ_HEAD(Rtl_vlan_list_entry_head, Rtl_Vlan_List_Entry) *list;
};

CTAILQ_HEAD(Rtl_vlan_list_inuse_head, Rtl_Vlan_List_Entry) rtl_vlan_list_inuse;
CTAILQ_HEAD(Rtl_vlan_list_free_head, Rtl_Vlan_List_Entry) rtl_vlan_list_free;

struct Rtl_Vlan_Table *table_rtl_vlan;
static int rtl_vlan_table_list_max;

static inline unsigned int Hash_Rtl_Vlan_Entry(unsigned short vid)
{
	register unsigned int hash;

	hash = vid;
	
	return hash & (rtl_vlan_table_list_max-1);
}

int rtl_initRtlVlanTable(int rtl_vlan_tbl_list_max, int rtl_vlan_tbl_entry_max)
{
	int i;

	table_rtl_vlan = (struct Rtl_Vlan_Table *)kmalloc(sizeof(struct Rtl_Vlan_Table), GFP_ATOMIC);
	if (table_rtl_vlan == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_vlan) \n");
		return -1;
	}
	CTAILQ_INIT(&rtl_vlan_list_inuse);
	CTAILQ_INIT(&rtl_vlan_list_free);

	rtl_vlan_table_list_max=rtl_vlan_tbl_list_max;
	table_rtl_vlan->list=(struct Rtl_vlan_list_entry_head *)kmalloc(rtl_vlan_table_list_max*sizeof(struct Rtl_vlan_list_entry_head), GFP_ATOMIC);

	if (table_rtl_vlan->list == NULL) {
		diag_printf("MALLOC Failed! (table_rtl_vlan list) \n");
		return -1;
	}
	for (i=0; i<rtl_vlan_table_list_max; i++) {
		CTAILQ_INIT(&table_rtl_vlan->list[i]);
	}

	for (i=0; i<rtl_vlan_tbl_entry_max; i++) {
		struct Rtl_Vlan_List_Entry *entry = (struct Rtl_Vlan_List_Entry *)kmalloc(sizeof(struct Rtl_Vlan_List_Entry), GFP_ATOMIC);
		if (entry == NULL) {
			diag_printf("MALLOC Failed! (Rtl_Vlan_List_Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&rtl_vlan_list_free, entry, tqe_link);
	}

	return 0;
}


struct Rtl_Vlan_List_Entry *rtl_lookupRtlVlanEntry(unsigned short vid)
{
	int s;
	unsigned int hash;
	struct Rtl_Vlan_List_Entry *entry;
	
	s = splimp();

	hash = Hash_Rtl_Vlan_Entry(vid);
	CTAILQ_FOREACH(entry, &table_rtl_vlan->list[hash], rtl_vlan_link) {
	       if ((entry->vlan_entry.vid == vid)&&(entry->vlan_entry.valid == 1))
		{
			splx(s);
			return entry;
		}
	}

	splx(s);
	return NULL;
}


int rtl_addRtlVlanEntry(unsigned short vid)
{
	int s;
	unsigned int hash;
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	hash = Hash_Rtl_Vlan_Entry(vid);

	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry)
		return RTL_VLANALREADYEXISTS;	//entry already exist
		
	s = splimp();
	if(!CTAILQ_EMPTY(&rtl_vlan_list_free)) {
		entry = CTAILQ_FIRST(&rtl_vlan_list_free);
		memset(&(entry->vlan_entry), 0, sizeof(rtl_vlan_entry_t));
		entry->vlan_entry.valid = 1;
		entry->vlan_entry.vid = vid;
		
		CTAILQ_REMOVE(&rtl_vlan_list_free, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&rtl_vlan_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&table_rtl_vlan->list[hash], entry, rtl_vlan_link);
		splx(s);
		return SUCCESS;
	}

	splx(s);
	return FAILED;
}

int rtl_delRtlVlanEntry(unsigned short vid)
{
	int s;
	unsigned int hash;
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	hash = Hash_Rtl_Vlan_Entry(vid);

	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_VLANALREADYDEL;	//entry already delete

	s = splimp();
	entry->vlan_entry.valid = 0;
	CTAILQ_REMOVE(&table_rtl_vlan->list[hash], entry, rtl_vlan_link);
	CTAILQ_REMOVE(&rtl_vlan_list_inuse, entry, tqe_link);
	CTAILQ_INSERT_TAIL(&rtl_vlan_list_free, entry, tqe_link);
	
	splx(s);
	return SUCCESS;
}

int rtl_flushRtlVlanEntry(void)
{
	int i, s;
	unsigned int hash;
	struct Rtl_Vlan_List_Entry *entry;

	s = splimp();
	for(i=0; i<rtl_vlan_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_rtl_vlan->list[i], rtl_vlan_link) {
			CTAILQ_REMOVE(&table_rtl_vlan->list[i], entry, rtl_vlan_link);
			CTAILQ_REMOVE(&rtl_vlan_list_inuse, entry, tqe_link);
			CTAILQ_INSERT_TAIL(&rtl_vlan_list_free, entry, tqe_link);
		}
	}

	splx(s);
	return SUCCESS;
}


int rtl_addRtlVlanMemPort(unsigned short vid, unsigned int portMask)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	

	entry->vlan_entry.memPortMask  |= portMask;
	entry->vlan_entry.untagPortMask |= portMask;
		
	return SUCCESS;
}

int rtl_delRtlVlanMemPort(unsigned short vid, unsigned int portMask)
{
	int s;
	unsigned int hash;
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;

	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	
	
	entry->vlan_entry.memPortMask  &= ~portMask;
	entry->vlan_entry.untagPortMask &= ~portMask;
	
	if(entry->vlan_entry.memPortMask == 0){
		hash = Hash_Rtl_Vlan_Entry(vid);
		s = splimp();
		entry->vlan_entry.valid = 0;
		CTAILQ_REMOVE(&table_rtl_vlan->list[hash], entry, rtl_vlan_link);
		CTAILQ_REMOVE(&rtl_vlan_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&rtl_vlan_list_free, entry, tqe_link);
		splx(s);
	}

	return SUCCESS;
}

int rtl_setRtlVlanPortTag(unsigned short vid, unsigned int portMask, int tag)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	

	if(tag)
		entry->vlan_entry.untagPortMask &= ~(entry->vlan_entry.memPortMask&portMask);
	else
		entry->vlan_entry.untagPortMask  |= (entry->vlan_entry.memPortMask&portMask);
	
	return SUCCESS;
}

int rtl_setRtlVlanForwardRule(unsigned int vid, int forwardRule)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	

	entry->vlan_entry.forward_rule = forwardRule;
	
	return SUCCESS;
}

int rtl_getRtlVlanForwardRule(unsigned short vid, int *forwardRule)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	

	 *forwardRule = entry->vlan_entry.forward_rule;
	
	return SUCCESS;
}

int rtl_setRtlVlanPortPvidByPortMask(unsigned short pvid, unsigned int portMask)
{
	int i;

	if(pvid < 1 ||pvid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(portMapInt[i].portMask == portMask){
			portMapInt[i].pvid = pvid;
			break;
		}
	}
	
	return SUCCESS;
}


unsigned int rtl_getRtlVlanPortMask(unsigned int vid)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	
	
	return entry->vlan_entry.memPortMask;
}

unsigned int rtl_getRtlVlanUntagPortMask(unsigned int vid)
{
	struct Rtl_Vlan_List_Entry *entry;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;
	
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry==NULL)
		return RTL_INVALIDVLANID;	
	
	
	return entry->vlan_entry.untagPortMask;
}

int rtl_setVlanPortPvidByDevName(unsigned short pvid, unsigned char *name)
{
	int i;
	
	if(pvid < 1 ||pvid > (VLAN_NUMBER-1))
		return RTL_INVALIDVLANID;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name))){
			portMapInt[i].pvid = pvid;
			break;
		}
	}
	
	return SUCCESS;
}

int rtl_getVlanPortPvidByDevName(unsigned char *name)
{
	int i;
	
	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name)))
			return portMapInt[i].pvid;
	}
	
	return 0;
}

int rtl_setVlanPortCfiByDevName(unsigned char cfi, unsigned char *name)
{
	int i;
	
	if((cfi !=0)&&(cfi!=1))
		return FAILED;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name))){
			portMapInt[i].cfi= cfi;
			break;
		}
	}
	
	return SUCCESS;
}

int rtl_getVlanPortCfiByDevName(unsigned char *name)
{
	int i;
	
	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name)))
			return portMapInt[i].cfi;
	}
	
	return 0;
}

int rtl_setVlanPortPriByDevName(unsigned char pri, unsigned char *name)
{
	int i;
	
	if(pri>7)
		return FAILED;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name))){
			portMapInt[i].priority = pri;
			break;
		}
	}
	
	return SUCCESS;
}

int rtl_getVlanPortPriByDevName(unsigned char *name)
{
	int i;
	
	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*(portMapInt[i].name) &&*/ (!strcmp(portMapInt[i].name, name)))
			return portMapInt[i].priority;
	}
	
	return 0;
}


int rtl_portNameConvertNetifName(unsigned char *portName, unsigned char *netifName)
{	
	int ret = SUCCESS;
	if(!netifName || !portName)
		return FAILED;
	
	if(!memcmp(portName, RTL_DRV_LAN_P0_NAME, 4))
		memcpy(netifName, RTL_DRV_LAN0_NETIF_NAME, sizeof(RTL_DRV_LAN0_NETIF_NAME));
	else if(!memcmp(portName, RTL_DRV_LAN_P1_NAME, 4))
		memcpy(netifName, RTL_DRV_LAN1_NETIF_NAME, sizeof(RTL_DRV_LAN1_NETIF_NAME));
	else if(!memcmp(portName, RTL_DRV_LAN_P2_NAME, 4))
		memcpy(netifName, RTL_DRV_LAN2_NETIF_NAME, sizeof(RTL_DRV_LAN2_NETIF_NAME));
	else if(!memcmp(portName, RTL_DRV_LAN_P3_NAME, 4))
		memcpy(netifName, RTL_DRV_LAN3_NETIF_NAME, sizeof(RTL_DRV_LAN3_NETIF_NAME));
	else if(!memcmp(portName, RTL_DRV_WAN_P4_NAME, 4))
		memcpy(netifName, RTL_DRV_WAN_NETIF_NAME, sizeof(RTL_DRV_WAN_NETIF_NAME));
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
	else if(!memcmp(portName, RTL_DRV_LAN_P7_NAME, 4))
	    memcpy(netifName, RTL_DRV_LAN7_P7_VNETIF_NAME, sizeof(RTL_DRV_LAN7_P7_VNETIF_NAME));
    #endif
	#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	else if(!memcmp(portName, RTL_DRV_LAN_P8_NAME, 4))
	    memcpy(netifName, RTL_DRV_LAN8_P8_VNETIF_NAME, sizeof(RTL_DRV_LAN8_P8_VNETIF_NAME));
	#endif
	else
		ret = FAILED;

	return ret;
}

int rtl_netifNameConvertportName(unsigned char *netifName, unsigned char *portName)
{
	int ret = SUCCESS;
	if(!netifName || !portName)
		return FAILED;
	
	if(!memcmp(netifName, RTL_DRV_LAN0_NETIF_NAME, 4))
		memcpy(portName, RTL_DRV_LAN_P0_NAME, sizeof(RTL_DRV_LAN_P0_NAME));
	else if(!memcmp(netifName, RTL_DRV_LAN1_NETIF_NAME, 4))
		memcpy(portName, RTL_DRV_LAN_P1_NAME, sizeof(RTL_DRV_LAN_P1_NAME));
	else if(!memcmp(netifName, RTL_DRV_LAN2_NETIF_NAME, 4))
		memcpy(portName, RTL_DRV_LAN_P2_NAME, sizeof(RTL_DRV_LAN_P2_NAME));
	else if(!memcmp(netifName, RTL_DRV_LAN3_NETIF_NAME, 4))
		memcpy(portName, RTL_DRV_LAN_P3_NAME, sizeof(RTL_DRV_LAN_P3_NAME));
	else if(!memcmp(netifName, RTL_DRV_WAN_NETIF_NAME, 4))
		memcpy(portName, RTL_DRV_WAN_P4_NAME, sizeof(RTL_DRV_WAN_P4_NAME));
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
	else if(!memcmp(netifName, RTL_DRV_LAN7_P7_VNETIF_NAME, 4))
	    memcpy(netifName, RTL_DRV_LAN_P7_NAME, sizeof(RTL_DRV_LAN_P7_NAME));
    #endif
	#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	else if(!memcmp(netifName, RTL_DRV_LAN8_P8_VNETIF_NAME, 4))
	    memcpy(netifName, RTL_DRV_LAN_P8_NAME, sizeof(RTL_DRV_LAN_P8_NAME));
	#endif
	else
		ret = FAILED;

	return ret;
}
void rtl_setPortDefVlanByDevName(unsigned short pVid, unsigned char pPri, unsigned char pCfi, unsigned char *name)
{
	unsigned char *nameTmp;
	unsigned char netifName[IFNAMSIZ];

	if(rtl_portNameConvertNetifName(name, netifName) == SUCCESS)
		nameTmp = netifName;
	else
		nameTmp = name;
	
	rtl_setVlanPortPvidByDevName(pVid, nameTmp);
	rtl_setVlanPortPriByDevName(pPri, nameTmp);
	rtl_setVlanPortCfiByDevName(pCfi, nameTmp);
}
unsigned int rtl_getPortMaskByDevName(unsigned char *name)
{
	int i;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(/*portMapInt[i].name&&*/ (!strcmp(portMapInt[i].name, name)))
			return portMapInt[i].portMask;
	}

	return 0;
}

unsigned char *rtl_getDevNameByPortMask(unsigned int portMask)
{
	int i;

	for(i=0; i<MAX_INTERFACE_NUM; i++)
	{
		if(portMapInt[i].portMask == portMask)
			return portMapInt[i].name;
	}

	return NULL;
}

int rtl_addRtlVlanMemberPortByDevName(unsigned short vid, unsigned char *name)
{
	int ret;
	unsigned int portMask;
	unsigned char *nameTmp;
	unsigned char netifName[IFNAMSIZ];

	if(rtl_portNameConvertNetifName(name, netifName) == SUCCESS)
		nameTmp = netifName;
	else
		nameTmp = name;
	
	portMask = rtl_getPortMaskByDevName(nameTmp);
	if(portMask == 0)
		return RTL_INVALIDVLANID;
	
	ret = rtl_addRtlVlanMemPort(vid, portMask);

	return ret;
}

int rtl_delRtlVlanMemPortByDevName(unsigned short vid, unsigned char *name)
{
	int ret;
	unsigned int portMask;
	unsigned char *nameTmp;
	unsigned char netifName[IFNAMSIZ];

	if(rtl_portNameConvertNetifName(name, netifName) == SUCCESS)
		nameTmp = netifName;
	else
		nameTmp = name;

	portMask = rtl_getPortMaskByDevName(nameTmp);
	if(portMask == 0)
		return RTL_INVALIDVLANID;
	
	ret = rtl_delRtlVlanMemPort(vid, portMask);

	return ret;
}

int rtl_setRtlVlanPortTagByDevName(unsigned short vid, unsigned char *name, int tag)
{
	int ret;
	unsigned int portMask;
	unsigned char *nameTmp;
	unsigned char netifName[IFNAMSIZ];
	
	if(rtl_portNameConvertNetifName(name, netifName) == SUCCESS)
		nameTmp = netifName;
	else
		nameTmp = name;
	
	portMask = rtl_getPortMaskByDevName(nameTmp);
	if(portMask == 0)
		return RTL_INVALIDVLANID;
	
	ret = rtl_setRtlVlanPortTag(vid, portMask, tag);

	return ret;
}

static inline int rtl_pvidIsTagByDevName(unsigned char *dev_name)
{
	int ret = true;
	struct Rtl_Vlan_List_Entry *entry;
	unsigned short  pvid = 0;
	unsigned int portMask = 0;

	portMask = rtl_getPortMaskByDevName(dev_name);
	pvid = rtl_getVlanPortPvidByDevName(dev_name);
	entry = rtl_lookupRtlVlanEntry(pvid);

	if((entry==NULL) || (entry->vlan_entry.valid==0) ||((entry->vlan_entry.memPortMask&portMask)==0))
		return ret;

	if(entry->vlan_entry.untagPortMask & portMask)
		ret = false;

	return ret;
}

static inline int rtl_portBasedVlanDecision(struct sk_buff *skb, unsigned char *dev_name)
{
	unsigned short  pvid = 0;
	unsigned char cfi = 0;
	unsigned char pri = 0;

	pvid = rtl_getVlanPortPvidByDevName(dev_name);
	cfi    = rtl_getVlanPortCfiByDevName(dev_name);
	pri    = rtl_getVlanPortPriByDevName(dev_name);
	if(pvid == 0)	//interface should default set pvid
		return FAILED;

	skb->tag.f.tpid =  htons(ETH_P_8021Q);
	skb->tag.f.pci =  (unsigned short) (((pri&0x7) << 13) |((cfi&0x1) << 12) |(pvid&0xfff));
	skb->tag.f.pci  =  htons(skb->tag.f.pci);

	return SUCCESS;
}
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
unsigned char BRCST_MAC[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
static inline int rtl_IsTagByVid(unsigned short vid, unsigned char *dev_name)
{
	struct Rtl_Vlan_List_Entry *entry;
	int ret = 0;
    unsigned short portMask = rtl_getPortMaskByDevName(dev_name);
    
	entry = rtl_lookupRtlVlanEntry(vid);
	if(entry && (entry->vlan_entry.valid) &&(entry->vlan_entry.memPortMask&portMask)){
		if(entry->vlan_entry.untagPortMask & portMask)
			ret = false;
		else
			ret = true;
	}

	return ret;
}

#endif
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
static inline int rtl_vlanTagIngressDecision(struct sk_buff *skb, unsigned char *dev_name, int is_wan, struct sk_buff **new_skb)
#else
static inline int rtl_vlanTagIngressDecision(struct sk_buff *skb, unsigned char *dev_name, int is_wan)
#endif
{
	struct vlan_tag tag;
	int ret = SUCCESS;
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    int forward_rule = 0;
    int taged = 0;
	if(new_skb)
		*new_skb = NULL;
    if (!is_wan)
        skb->flag_src = 1;
    #endif

	memcpy(&tag, skb->data+ETH_ALEN*2, VLAN_HLEN);
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    if (tag.f.tpid == htons(ETH_P_8021Q))
    {
        taged = rtl_IsTagByVid(ntohs(skb->tag.f.pci)&0xfff, dev_name);
    }
    else
    {
        taged = rtl_pvidIsTagByDevName(dev_name);
    }
    if (is_wan && taged)
        taged = 1;
    else
        taged =0;
    #endif
	
    #if 0
    if (taged && tag.f.tpid != htons(ETH_P_8021Q)) {
		if(!(skb->data[0] & 0x01))
			diag_printf("<Drop> due to packet w/o tag but port-tag is enabled!\n");
		return 1;
	}
    #endif
    
	if(tag.f.tpid == htons(ETH_P_8021Q)){	//tag exist in the incoming pkt	
        //diag_printf("%s %d tag exist in the incoming pkt\n", __FUNCTION__, __LINE__);
        memcpy(&skb->tag, &tag, VLAN_HLEN);
		STRIP_TAG(skb);
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if (is_wan)
        {
            struct Rtl_Vlan_List_Entry *entry;
            entry = rtl_lookupRtlVlanEntry(ntohs(tag.f.pci)&0xfff);
            if((entry != NULL) && (entry->vlan_entry.valid != 0))
            {
                if (entry->vlan_entry.forward_rule == LAN_WAN_BRIDGE)
                {
                    #if defined(CONFIG_RTL_819X_SWCORE)
                    skb->dev = rtl_get_virtual_dev_for_bridge_vlan(); //return virtual interface
                    #endif
                    //diag_printf("lan wan bridge\n");
                    if (skb->dev == NULL)
                        return 1;
					skb->forward_rule = LAN_WAN_BRIDGE;
                }
                else if (entry->vlan_entry.forward_rule == LAN_WAN_NAPT)
                {
                    //diag_printf("lan wan napt\n");
                    skb->forward_rule = LAN_WAN_NAPT;
                    return SUCCESS;
                }
                else
                {
                    //diag_printf("enable bridge vlan but no forward rule.\n");
                    return 1;
                }
            }
            else 
                return FAILED;
        }
        else
        {
            rtl_getRtlVlanForwardRule(ntohs(skb->tag.f.pci)&0xfff, &forward_rule);
            skb->forward_rule = forward_rule;
            skb->taged = rtl_IsTagByVid(ntohs(skb->tag.f.pci)&0xfff, dev_name); 
        }
        #endif
	}else if(is_wan==0){	    /*wan port rx untag skb will not add pvid to skb->tag now*/

		#if 1	/*If pvid is tag, rx untag pkt will be dropped*/
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if (taged == true)
        #else
		if(rtl_pvidIsTagByDevName(dev_name)==true)
        #endif
			return FAILED;
		#endif
		
		if(currVlanDecisionMode & (MAC_BASED_VLAN_MODE |SUBNET_BASED_VLAN_MODE |SUBNET_BASED_VLAN_MODE)){
			/*high priority vlan decision, to do list, so now it will return FAILED!*/
			ret = FAILED;
		}else if(currVlanDecisionMode & PORT_BASED_VLAN_MODE){
			ret = rtl_portBasedVlanDecision(skb, dev_name);
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
            rtl_getRtlVlanForwardRule(ntohs(skb->tag.f.pci)&0xfff, &forward_rule);
            skb->forward_rule = forward_rule;
            skb->taged = rtl_pvidIsTagByDevName(dev_name);
            #endif
		}
	}
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    else if (is_wan)
    {
        unsigned char wan_macaddr[6] = {0};
	 #if defined(CONFIG_RTL_819X_SWCORE)
        rtl_getWanMacAddr(wan_macaddr);
	 #endif
        if (!memcmp(wan_macaddr, skb->data, 6))/*unicast for nat*/
        { 
			skb->forward_rule = LAN_WAN_NAPT;
			//diag_printf("%s %d unicast for nat \n", __FUNCTION__, __LINE__);   
        }
        else if (skb->data[0] & 0x01) /*multicast and broadcast */
        { 
            //diag_printf("%s %d multicast and broadcast \n", __FUNCTION__, __LINE__);
            unsigned short pvid = rtl_getVlanPortPvidByDevName(dev_name);
            skb->flag_src = 1;
            rtl_getRtlVlanForwardRule(pvid, &forward_rule);
            skb->forward_rule = forward_rule;
            skb->index = 0;
            skb->taged = rtl_pvidIsTagByDevName(dev_name);

			#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
			if(rtl_isEthPassthruFrame(skb->data) && new_skb)
			#else
            if (new_skb) 
			#endif
            {
            #if defined(CONFIG_RTL_8197F)
				//ip_input may change data, but mbuf no copy, leading packets error
				*new_skb = skb_copy(skb, GFP_ATOMIC);
			#else
                *new_skb = skb_clone(skb, GFP_ATOMIC);
			#endif
                if (*new_skb == NULL) {
                    diag_printf("skb_clone() failed!\n");
                }
                else 
                {
                    #if defined(CONFIG_RTL_819X_SWCORE)
                    (*new_skb)->dev = rtl_get_virtual_dev_for_bridge_vlan();
		     #endif
                    if((*new_skb)->dev == NULL)
                        return 2;
                    //pvid = rtl_getVlanPortPvidByDevName((*new_skb)->dev->name);
                    //rtl_getRtlVlanForwardRule(pvid, &forward_rule);
                    //(*new_skb)->forward_rule = forward_rule;
                    (*new_skb)->forward_rule = LAN_WAN_BRIDGE;
                    (*new_skb)->index=1;
                }
            }
        }
        else if(memcmp(BRCST_MAC, skb->data, 6)){    /* unicast for bridge */
		#if defined(CONFIG_RTL_819X_SWCORE)
            skb->dev = rtl_get_virtual_dev_for_bridge_vlan();
			skb->forward_rule = LAN_WAN_BRIDGE;
		#endif
            if(skb->dev == NULL)
                return 1;                    
        }
    }
    #endif
    	
	return ret;
}

static inline int rtl_vlanIngressFilter(struct sk_buff *skb, unsigned char *dev_name, int is_wan)
{
	struct Rtl_Vlan_List_Entry *entry;
	unsigned short vid = ntohs(skb->tag.f.pci)&0xfff;
	unsigned int portMask = rtl_getPortMaskByDevName(dev_name);
	
	/*Because untag pkt rcv by wan port will not add wan pvid, so not to check ingress filter*/
	if(is_wan)
		return SUCCESS;

	if(vid < 1 ||vid > (VLAN_NUMBER-1))
		return FAILED;

	entry = rtl_lookupRtlVlanEntry(vid);
	
	if((entry==NULL) || (entry->vlan_entry.valid==0) ||((entry->vlan_entry.memPortMask&portMask)==0))
		return FAILED;

	return SUCCESS;
}


#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
int  rtl_vlanIngressProcess(struct sk_buff *skb, unsigned char *dev_name, struct sk_buff **new_skb)
#else
int rtl_vlanIngressProcess(struct sk_buff *skb, unsigned char *dev_name)
#endif
{
	int ret = FAILED;
	int isWanDev = 0;
	
 	#if defined(CONFIG_RTL_819X_SWCORE)
	isWanDev = rtl_isWanDevDecideByName(dev_name);    
 	#endif
	
    	#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    	ret = rtl_vlanTagIngressDecision(skb, dev_name, isWanDev, new_skb);
	if(ret != SUCCESS)
		return ret;
    	#else
	ret = rtl_vlanTagIngressDecision(skb, dev_name, isWanDev);
	if(ret == FAILED)
		return RTL_ERRVLANINGRESSDECISION;
    	#endif

	ret = rtl_vlanIngressFilter(skb, dev_name, isWanDev);
	if(ret == FAILED)
		return RTL_ERRVLANINGRESSFILTER;

	return SUCCESS;
}

static inline int rtl_decideWanPortTag(unsigned char *dev_name)
{
	struct Rtl_Vlan_List_Entry *entry;
	unsigned int portMask;
	int ret = 0;
	unsigned short wanPvid = 0;

	wanPvid = rtl_getVlanPortPvidByDevName(dev_name);
	portMask = rtl_getPortMaskByDevName(dev_name);
	entry = rtl_lookupRtlVlanEntry(wanPvid);
	if(entry && (entry->vlan_entry.valid) &&(entry->vlan_entry.memPortMask&portMask)){
		if(entry->vlan_entry.untagPortMask & portMask)
			ret = false;
		else
			ret = true;
	}

	#if 0
	if(vid == wanPvid){
		/*AP->wan*/
		if(wanApOriTag == true)
			ret = true;
		else
			ret = false;
	}else{
		/*lan->wan*/
		if(wanForwardTag == true)
			ret = true;
		else
			ret = false;
	}
	#endif

	return ret;
}

static inline int rtl_vlanTagEgressDecision(struct sk_buff *skb, unsigned char *dev_name, int is_wan)
{
	int ret = SUCCESS;
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    unsigned char lan_macaddr[6] = {0};
    int forwardRule = 0;
    
    if(skb->tag.f.tpid == htons(ETH_P_8021Q))// lan<->wan forward
        rtl_getRtlVlanForwardRule(ntohs(skb->tag.f.pci)&0xfff, &forwardRule);     
    #endif
    
    if(skb->tag.f.tpid != htons(ETH_P_8021Q)){
		if(currVlanDecisionMode & (MAC_BASED_VLAN_MODE |SUBNET_BASED_VLAN_MODE |SUBNET_BASED_VLAN_MODE)){
			/*high priority vlan decision, to do list, so now it will return FAILED!*/
			return FAILED;
		}else if(currVlanDecisionMode & PORT_BASED_VLAN_MODE){
			ret = rtl_portBasedVlanDecision(skb, dev_name);
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
            rtl_getRtlVlanForwardRule(ntohs(skb->tag.f.pci)&0xfff, &forwardRule);
            #endif
		}
	}
    else if(is_wan && (wanVlanTagMode == REPALCE_ORI_VLAN_MODE)){
		ret = rtl_portBasedVlanDecision(skb, dev_name);	/*In this wanVlanTagMode, all pkt tx to wan will tagged with wan pvid*/
	}
    
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    #if 1
    if (is_wan)
    {
       if (skb->taged&0x1)
        skb->taged = 1;
    }
    else
        skb->taged = 0;
    #endif
    #if defined(CONFIG_RTL_819X_SWCORE)
    rtl_getLanMacAddr(lan_macaddr);
    #endif
    if((!memcmp(lan_macaddr,skb->data+6, 6)) && (forwardRule==LAN_WAN_BRIDGE) && (skb->data[37] == 68))
	{
	
		return FAILED; /* discard dhcp response from lan dhcpserver if  
		                            this vlan group enable bridge vlan */
	}
    #endif
		
	return ret;
}

static inline int rtl_vlanEgressFilter(struct sk_buff *skb, unsigned char *dev_name, int *tag, int is_wan)
{
	struct Rtl_Vlan_List_Entry *entry;
	unsigned short vid;
	unsigned int portMask;
	int forward_rule = 0;
	if(!is_wan && skb->forward_rule !=0)
	{
		vid = rtl_getVlanPortPvidByDevName(dev_name) & 0xfff;
		rtl_getRtlVlanForwardRule(vid, &forward_rule);
		if(skb->forward_rule != forward_rule)
		{
			#if 0
			if(forward_rule == 1)
				panic_printk("NAT->Bridge, drop!\n");
			else if(forward_rule == 2)
				panic_printk("Bridge->NAT, drop!\n");
			else
				panic_printk("Wrong case, drop!\n");
			#endif

			return FAILED;
		}
	}

	/*dest dev should be in the skb->tag group*/
	if(skb->tag.f.tpid == htons(ETH_P_8021Q)){
		vid = ntohs(skb->tag.f.pci)&0xfff;
		portMask = rtl_getPortMaskByDevName(dev_name);
        
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        if (is_wan && skb->flag_src)
        {
            /* use soure vlan info decide tag or not */
            if (skb->taged)
                *tag = true;
            else
                *tag = false;
            return SUCCESS;
        }
        else if (is_wan)
        {
            *tag = rtl_decideWanPortTag(dev_name); 
            //diag_printf("%s %d dev_name %s *tag %d vid %d skb->flag_src %d skb->taged %d \n", __FUNCTION__, __LINE__, dev_name, *tag, vid,skb->flag_src ,skb->taged);
            return SUCCESS;
            
        }
        #else
        if(is_wan){
			*tag = rtl_decideWanPortTag(dev_name);
			return SUCCESS;
		}
        #endif

		if(vid < 1 ||vid > (VLAN_NUMBER-1))
			return FAILED;

		entry = rtl_lookupRtlVlanEntry(vid);

		if((entry==NULL) || (entry->vlan_entry.valid == 0) ||((entry->vlan_entry.memPortMask&portMask) == 0))
			return FAILED;
        
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        #else
		if(entry->vlan_entry.untagPortMask & portMask)            
        #endif
			*tag = false;
			
	}

	return SUCCESS;
}

static int rtl_vlanEgressInsertVlanTag(struct sk_buff *skb, unsigned char *dev_name, int tag, int is_wan, int wlan_pri)
{
	struct vlan_tag tag_tmp;
	

	#if 0	//for test
	if(*((unsigned short*)(skb->data+(ETH_ALEN<<1))) == ((unsigned short)0x0806))
	{
		tag = false;
	}
	#endif

	if(wlan_pri)
		skb->cb[0] = '\0';		// for WMM priority
	
	if(tag == false){	//untag xmit
		if(*((unsigned short*)(skb->data+(ETH_ALEN<<1))) == htons(ETH_P_8021Q)){
			memmove(skb->data + VLAN_HLEN, skb->data, 2*ETH_ALEN);
			skb_pull(skb, VLAN_HLEN);
		}

		goto OUT;
	}
	
	memcpy(&tag_tmp, skb->data+ETH_ALEN*2, VLAN_HLEN);
	if (tag_tmp.f.tpid !=  htons(ETH_P_8021Q)) { // tag not existed, insert tag
		if (skb_headroom(skb) < VLAN_HLEN && skb_cow(skb, VLAN_HLEN) !=0 ) {
			diag_printf("%s-%d: error! (skb_headroom(skb) == %d < 4). Enlarge it!\n",
			__FUNCTION__, __LINE__, skb_headroom(skb));
			#ifndef __ECOS
			while (1) ;
			#endif
			return FAILED;
		}
		skb_push(skb, VLAN_HLEN);
		memmove(skb->data, skb->data+VLAN_HLEN, ETH_ALEN*2);
	}

	if(skb->tag.f.tpid == htons(ETH_P_8021Q)){	//here skb->tag should be set
		memcpy(skb->data+ETH_ALEN*2, &skb->tag, VLAN_HLEN);
	}
	else
		return FAILED;

OUT:
	if (wlan_pri)
		skb->cb[0] = (unsigned char)((ntohs(skb->tag.f.pci)>>13)&0x7);

	return SUCCESS;
}


int rtl_vlanEgressProcess(struct sk_buff *skb, unsigned char *dev_name, int wlan_pri)
{
	int tag = true;
	int ret = FAILED;
	int isWanDev = 0;
	
	 #if defined(CONFIG_RTL_819X_SWCORE)
	isWanDev = rtl_isWanDevDecideByName(dev_name);
	#endif
		
	ret = rtl_vlanTagEgressDecision(skb, dev_name, isWanDev);
	if(ret == FAILED)
		return RTL_ERRVLANINGRESSDECISION;

	ret = rtl_vlanEgressFilter(skb, dev_name, &tag, isWanDev);
	if(ret == FAILED)
		return RTL_ERRVLANEGRESSFILTER;

	ret = rtl_vlanEgressInsertVlanTag(skb, dev_name, tag, isWanDev, wlan_pri);
	if(ret == FAILED)
		return RTL_ERRVLANEGRESSINSERTVLANTAG;

	return SUCCESS;
}

int rtl_vlanTableShow(void)
{
	int j;
	unsigned char portName[IFNAMSIZ];
	struct Rtl_Vlan_List_Entry *entry;

	diag_printf("%s\n", "RTL VLAN Table:");
	{
		CTAILQ_FOREACH(entry, &rtl_vlan_list_inuse, tqe_link) {
			if(entry->vlan_entry.valid == 0)
				continue;

			diag_printf("  VID[%d] ", entry->vlan_entry.vid);
			diag_printf("\n\tmember interface:");

			for( j = 0; j < MAX_INTERFACE_NUM; j++ )
			{
				if ((entry->vlan_entry.memPortMask & portMapInt[j].portMask) && portMapInt[j].name){
					if(rtl_netifNameConvertportName(portMapInt[j].name, portName)==SUCCESS)
						diag_printf("%s ", portName);
					else
						diag_printf("%s ", portMapInt[j].name);
				}
			}

			diag_printf("\n\tUntag member interface:");				

			for( j = 0; j < MAX_INTERFACE_NUM; j++ )
			{
				if((entry->vlan_entry.untagPortMask & portMapInt[j].portMask) && portMapInt[j].name){
					if(rtl_netifNameConvertportName(portMapInt[j].name, portName)==SUCCESS)
						diag_printf("%s ", portName);
					else
						diag_printf("%s ", portMapInt[j].name);
				}
			}
			diag_printf("\n");
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
            unsigned char tmprule[12] = {0};
            if (entry->vlan_entry.forward_rule == 1)
                sprintf(tmprule, "%s", "NAT");
            else if (entry->vlan_entry.forward_rule == 2)
                sprintf(tmprule, "%s", "Bridge");
            else
                sprintf(tmprule, "%s", "no rule");
            diag_printf("\tforward rule: %s\n", tmprule);
            #endif
		}
	}	

	return 0;
}

void rtl_showPortDefVlanInfo(void)
{
	int i;
	unsigned char *nameTmp;
	unsigned char portName[IFNAMSIZ];
	diag_printf("%s\n", "PVID SETTING:");

	for(i=0; i<MAX_INTERFACE_NUM-5; i++)
	{
		if(rtl_netifNameConvertportName(portMapInt[i].name, portName)==SUCCESS)
			nameTmp = portName;
		else
			nameTmp = portMapInt[i].name;
		
		diag_printf("[%d. %s]: pVid is %d, pPri is %d, pCfi is %d\n", i, nameTmp, 
			portMapInt[i].pvid, portMapInt[i].priority, portMapInt[i].cfi);
	}
}

int rtl_setVlanInfo(unsigned int argc, unsigned char *argv[])
{
	int arg;
	int ret = SUCCESS;
	char tmpBuff[32];
	rtl_vlan_entry_t vlan_entry;
    int vid = 0;
    
     /* rtl_vlan set-vid 10 member lan0 lan1 lan2 tagmember lan0 fwdrule Bridge */
	 memset(&vlan_entry, 0, sizeof(vlan_entry));	 
	 arg = 0;
	 while (arg<argc)
	 {
		if ((!strcmp(argv[arg], "set-vid"))) {
			if (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
				sscanf(tmpBuff, "%d", &vid);
                vlan_entry.vid = vid & 0xffff;
                ret = rtl_addRtlVlanEntry(vlan_entry.vid);
				if (ret == RTL_INVALIDVLANID || ret == FAILED) {
					diag_printf("Error: rtl_addRtlVlanEntry vlan id %d \n", vlan_entry.vid);
					ret = FAILED;
					goto OUT;
				}
				arg++;
			} else {
				ret = FAILED;
				goto OUT;
			}
		} else if ((!strcmp(argv[arg], "member"))) {
			    while (argv[++arg] && strcmp(argv[arg], "tagmember")!=0) {
    				strcpy(tmpBuff, argv[arg]);
    				ret = rtl_addRtlVlanMemberPortByDevName(vlan_entry.vid, tmpBuff);
                    if (ret == RTL_INVALIDVLANID)
                    {
    					diag_printf("Error: rtl_addRtlVlanMemberPortByDevName %s\n", tmpBuff);
    					ret = FAILED;
    					goto OUT;
    				}
			    }
		} else if ((!strcmp(argv[arg], "tagmember"))) {
		    	while (argv[++arg] && strcmp(argv[arg], "fwdrule")!=0) {
    				strcpy(tmpBuff, argv[arg]);
                    ret = rtl_setRtlVlanPortTagByDevName(vlan_entry.vid, tmpBuff, 1);
                    if (ret == RTL_INVALIDVLANID)
                    {
    					diag_printf("Error: rtl_setRtlVlanPortTagByDevName %s\n", tmpBuff);
    					ret = FAILED;
    					goto OUT;
    				}
			    }

		} else if ((!strcmp(argv[arg], "fwdrule"))) {
			if (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
				if (!strcmp(tmpBuff, "NAT")) {
					vlan_entry.forward_rule = 1;//nat mode
				} else if (!strcmp(tmpBuff, "Bridge")) {
					vlan_entry.forward_rule = 2;//bridge mode
				} else {
					diag_printf("Error: fwdrule %s is not suppported\n", tmpBuff);
					ret = FAILED;
					goto OUT;
				}                
				arg++;
			} else {
                vlan_entry.forward_rule = 1;//default nat
			}
            ret = rtl_setRtlVlanForwardRule(vlan_entry.vid, vlan_entry.forward_rule);
            if (ret == RTL_INVALIDVLANID)
                goto OUT;
		}  
	 }
		
OUT:
	return ret;
}

int rtl_setVlanPvidInfo(unsigned int argc, unsigned char *argv[])
{
	int arg;
	int ret = SUCCESS;
	char tmpBuff[32];
    unsigned short pvid = 0;
    int temp = 0;
    unsigned char defpriority = 0, defcfi = 0;

    /*  rtl_vlan set-pvid 10  priority 7 cfi 1 member lan0 lan1 lan2  */
	 arg = 0;
	 while (arg<argc)
	 {
		if ((!strcmp(argv[arg], "set-pvid"))) {
			if (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
				sscanf(tmpBuff, "%hu", &pvid);
				arg++;
			} else {
				ret = FAILED;
				goto OUT;
			}
		}
        else if ((!strcmp(argv[arg], "priority")))
        {
            if (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
				sscanf(tmpBuff, "%d", &temp);
                defpriority = temp & 0xff;
				arg++;
			} else {
				ret = FAILED;
				goto OUT;
			}
        }
        else if ((!strcmp(argv[arg], "cfi")))
        {
            if (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
				sscanf(tmpBuff, "%d", &temp);
                defcfi = temp & 0xff;
				arg++;
			} else {
				ret = FAILED;
				goto OUT;
			}
        }
        else if ((!strcmp(argv[arg], "member"))) {
		    while (argv[++arg]) {
				strcpy(tmpBuff, argv[arg]);
                rtl_setPortDefVlanByDevName(pvid, defpriority, defcfi, tmpBuff);
		    }
		} 
	 }
		
OUT:
	return ret;
}

int rtl_delVlanInfo(unsigned int argc, unsigned char *argv[])
{
	int arg;
	int ret = SUCCESS;
	char tmpBuff[32];
    unsigned short vid = 0;

    /*  eg:rtl_vlan del vid 10 */
    if (argc != 3)
        return ret;
    arg = 1;
	if ((!strcmp(argv[arg], "vid"))) {
		if (argv[++arg]) {
			strcpy(tmpBuff, argv[arg]);
			sscanf(tmpBuff, "%hu", &vid);
            rtl_delRtlVlanEntry(vid);
			arg++;
		} else {
			ret = FAILED;
			goto OUT;
		}
	}
		
OUT:
	return ret;
}
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
int rtl_addCustomPassthroughVlanInfo(void)
{
	int j;
	unsigned char portName[IFNAMSIZ];
	unsigned short wan_pvid = 0;
	unsigned int portmask = 0, wan_portmask = 0;
	
	struct Rtl_Vlan_List_Entry *entry;

	//find wan port pvid
	for( j = 0; j < MAX_INTERFACE_NUM; j++ )
	{
		if ((!memcmp(portMapInt[j].name, RTL_DRV_WAN_NETIF_NAME, strlen(RTL_DRV_WAN_NETIF_NAME))))
		{
			wan_pvid = portMapInt[j].pvid;
			break;
		}
	}
	//cannot find, error happened.
	if (j == MAX_INTERFACE_NUM)
	{
		return FAILED;
	}

	portmask = rtl_getPortMaskByDevName(RTL_DRV_LAN8_P8_VNETIF_NAME);
	wan_portmask = rtl_getPortMaskByDevName(RTL_DRV_WAN_NETIF_NAME);
	
	if (portmask == 0 || wan_portmask == 0)
	{
		return FAILED;
	}
	//diag_printf("%s %d wan_pvid=%d portmask=0x%x wan_portmask=0x%x \n", __FUNCTION__, __LINE__, wan_pvid, portmask, wan_portmask);
	{
		CTAILQ_FOREACH(entry, &rtl_vlan_list_inuse, tqe_link) {
			if(entry->vlan_entry.valid == 0)
				continue;
			if (entry->vlan_entry.vid == wan_pvid)
			{
				//diag_printf("%s %d vid=%d portmask=0x%x untagmsk=0x%x \n", __FUNCTION__, __LINE__, entry->vlan_entry.vid, entry->vlan_entry.memPortMask, entry->vlan_entry.untagPortMask);
				//add peth0's portmask to wan vlan group
				entry->vlan_entry.memPortMask |= portmask;
				if (entry->vlan_entry.untagPortMask & wan_portmask)
				{
					entry->vlan_entry.untagPortMask |= portmask;
				}
			}
		}
	}
	rtl_setVlanPortPvidByDevName(wan_pvid, RTL_DRV_LAN8_P8_VNETIF_NAME);
		
	return 0;
}
#endif

int rtl_initRtlVlan(void)
{
	rtl_initRtlVlanTable(RTL_VALN_TBL_LIST_MAX, RTL_VALN_TBL_ENTRY_MAX);

	#if 0	//test
	rtl_addRtlVlanEntry(RTL_LAN_DEFAULT_PVID);
	rtl_addRtlVlanMemPort(RTL_LAN_DEFAULT_PVID, 0x2f);
	rtl_setRtlVlanPortTag(RTL_LAN_DEFAULT_PVID, RTL_LANPORT_MASK_3, 1);

	rtl_addRtlVlanEntry(100);
	rtl_addRtlVlanMemPort(100, 0x2f);
	rtl_setRtlVlanPortTag(100, RTL_LANPORT_MASK_3, 1);

	rtl_addRtlVlanEntry(RTL_WAN_DEFAULT_PVID);
	rtl_addRtlVlanMemPort(RTL_WAN_DEFAULT_PVID, 0x10);
	#endif

	#if defined(__ECOS)&&!defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
	if(rtl_initTagForNatdTable(TAG_TABLE_FOR_NATD_LIST_MAX, TAG_TABLE_FOR_NATD_ENTRY_MAX)!=0) {
		diag_printf("init_vlan_tag_table_for_natd Failed!\n");
	}

	callout_init(&tag_for_natd_tbl_timer);
	callout_reset(&tag_for_natd_tbl_timer, TAG_TABLE_FOR_NATD_CHECK , rtl_tagForNatdTimeout, 0);
	#endif
	
	return SUCCESS;
}


/*I will add wan port in every vid group, and further I will set wan port is nat or bridge, tag or not!*/


#if defined(__ECOS)&&!defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
static inline unsigned int Hash_Vlan_Tag_For_Natd_Entry(unsigned short dir, unsigned short id, unsigned short protocol)
{
	register unsigned int hash;

	hash = dir;
	hash ^= id;
	hash ^= protocol;
	
	return (tag_for_natd_table_list_max-1) & (hash ^ (hash >> 12));
}

int rtl_addTagForNatdEntry(struct ip_info *ipInfo, struct vlan_tag tag)
{
	int s;
	unsigned int hash;
	struct Tag_For_Natd_List_Entry *entry;

	s = splimp();
	hash = Hash_Vlan_Tag_For_Natd_Entry(ipInfo->dir, ipInfo->id, ipInfo->protocol);
	if(!CTAILQ_EMPTY(&tag_for_natd_list_free)) {
		entry = CTAILQ_FIRST(&tag_for_natd_list_free);
		entry->valid = 1;
		entry->course = ipInfo->dir;
		entry->protocol	= ipInfo->protocol;
		entry->id	= ipInfo->id;
		entry->sIp	= ipInfo->sip;
		entry->dIp	= ipInfo->dip;
		entry->last_used = ipInfo->now;
		entry->tag.v	= tag.v;

		CTAILQ_REMOVE(&tag_for_natd_list_free, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&tag_for_natd_list_inuse, entry, tqe_link);
		CTAILQ_INSERT_TAIL(&table_tag_for_natd->list[hash], entry, tag_for_natd_link);
		splx(s);
		return SUCCESS;
	}

	splx(s);
	return FAILED;
}

int rtl_lookupTagForNatdEntry(struct ip_info *ipInfo, struct vlan_tag *tag)
{
	int s;
	unsigned int hash;
	struct Tag_For_Natd_List_Entry *entry;
	
	s = splimp();

	hash = Hash_Vlan_Tag_For_Natd_Entry(ipInfo->dir, ipInfo->id, ipInfo->protocol);
	CTAILQ_FOREACH(entry, &table_tag_for_natd->list[hash], tag_for_natd_link) {
	   if ((entry->valid==1) &&(entry->course==ipInfo->dir) &&
		((entry->dIp==ipInfo->dip) || (entry->sIp==ipInfo->sip))&&
		(entry->protocol==ipInfo->protocol)&&(entry->id==ipInfo->id))
		{
			entry->last_used = ipInfo->now;
			tag->v = entry->tag.v;

			/*The entry will be used for one time, after used, the entry can be delete*/
			entry->valid = 0;
			CTAILQ_REMOVE(&table_tag_for_natd->list[hash], entry, tag_for_natd_link);
			CTAILQ_REMOVE(&tag_for_natd_list_inuse, entry, tqe_link);
			CTAILQ_INSERT_TAIL(&tag_for_natd_list_free, entry, tqe_link);
			splx(s);
			return SUCCESS;
		}
	}

	splx(s);
	return FAILED;
}

int rtl_delTagForNatdEntry(void)
{
	int i, s;
	unsigned int hash;
	struct Tag_For_Natd_List_Entry *entry;
	unsigned long now = (unsigned long)jiffies;
	
	s = splimp();
	
	for(i=0; i<tag_for_natd_table_list_max; i++){
		CTAILQ_FOREACH(entry, &table_tag_for_natd->list[i], tag_for_natd_link) {
			if ((entry->valid==1) && ((entry->last_used+TAG_TABLE_FOR_NATD_TIMEOUT*HZ) < now))
			{
				entry->valid = 0;
				CTAILQ_REMOVE(&table_tag_for_natd->list[hash], entry, tag_for_natd_link);
				CTAILQ_REMOVE(&tag_for_natd_list_inuse, entry, tqe_link);
				CTAILQ_INSERT_TAIL(&tag_for_natd_list_free, entry, tqe_link);
				splx(s);
				return SUCCESS;
			}
		}
	}

	splx(s);
	return FAILED;
}


void rtl_showTagForNatdEntry(void)
{
	struct Tag_For_Natd_List_Entry *ep;

	unsigned long now = (unsigned long)jiffies;
	CTAILQ_FOREACH(ep, &tag_for_natd_list_inuse, tqe_link) {
		diag_printf("~TagForNatd: [dir: %d] [protocol: %d] [valid: %d] [id: %d] sip=0x%08X dip=0x%08X last_used=%d now=%d [tag: 0x%8X]\n",
			ep->course, ep->protocol, ep->valid, ep->id, ep->sIp, ep->dIp, ep->last_used, now, ep->tag.v);
	}
}

void rtl_tagForNatdTimeout(void)
{
	rtl_delTagForNatdEntry();
	callout_reset(&tag_for_natd_tbl_timer,TAG_TABLE_FOR_NATD_CHECK ,rtl_tagForNatdTimeout, 0);
}


int rtl_initTagForNatdTable(int tag_tbl_for_natd_list_max, int tag_tbl_for_natd_entry_max)
{
	int i;

	table_tag_for_natd = (struct Tag_For_Natd_Table *)kmalloc(sizeof(struct Tag_For_Natd_Table), GFP_ATOMIC);
	if (table_tag_for_natd == NULL) {
		diag_printf("MALLOC Failed! (table_tag_for_natd) \n");
		return -1;
	}
	CTAILQ_INIT(&tag_for_natd_list_inuse);
	CTAILQ_INIT(&tag_for_natd_list_free);

	tag_for_natd_table_list_max=tag_tbl_for_natd_list_max;
	table_tag_for_natd->list=(struct Tag_for_natd_list_entry_head *)kmalloc(tag_for_natd_table_list_max*sizeof(struct Tag_for_natd_list_entry_head), GFP_ATOMIC);
	if (table_tag_for_natd->list == NULL) {
		diag_printf("MALLOC Failed! (table_tag_for_natd list) \n");
		return -1;
	}
	for (i=0; i<tag_for_natd_table_list_max; i++) {
		CTAILQ_INIT(&table_tag_for_natd->list[i]);
	}

	for (i=0; i<tag_tbl_for_natd_entry_max; i++) {
		struct Tag_For_Natd_List_Entry *entry_path = (struct Tag_For_Natd_List_Entry *)kmalloc(sizeof(struct Tag_For_Natd_List_Entry), GFP_ATOMIC);
		if (entry_path == NULL) {
			diag_printf("MALLOC Failed! (Tag For Natd Table Entry) \n");
			return -2;
		}
		CTAILQ_INSERT_TAIL(&tag_for_natd_list_free, entry_path, tqe_link);
	}

	return 0;
}

#endif

