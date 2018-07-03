#ifndef __RTL_VLAN_H
#define __RTL_VLAN_H
/*
* Copyright c                  Realtek Semiconductor Corporation, 2012  
* All rights reserved.
* 
* Program : Software Vlan process
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/

#define VLAN_NUMBER	4096
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT) && defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#define MAX_INTERFACE_NUM	17
#elif defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT) || defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#define MAX_INTERFACE_NUM	16
#else
#define MAX_INTERFACE_NUM	15
#endif
#define RTL_VALN_TBL_LIST_MAX	16
#define RTL_VALN_TBL_ENTRY_MAX	16

#define RTL_DRV_LAN0_NETIF_NAME "eth0"
#define RTL_DRV_LAN1_NETIF_NAME "eth2"
#define RTL_DRV_LAN2_NETIF_NAME "eth3"
#define RTL_DRV_LAN3_NETIF_NAME "eth4"
#define RTL_DRV_WAN_NETIF_NAME "eth1"
#define RTL_DRV_WLAN_P0_NETIF_NAME "wlan0"
#define RTL_DRV_WLAN_P0_VA0_NAME "wlan0-va0"
#define RTL_DRV_WLAN_P0_VA1_NAME "wlan0-va1"
#define RTL_DRV_WLAN_P0_VA2_NAME "wlan0-va2"
#define RTL_DRV_WLAN_P0_VA3_NAME "wlan0-va3"
#define RTL_DRV_WLAN_P1_NETIF_NAME "wlan1"
#define RTL_DRV_WLAN_P1_VA0_NAME "wlan1-va0"
#define RTL_DRV_WLAN_P1_VA1_NAME "wlan1-va1"
#define RTL_DRV_WLAN_P1_VA2_NAME "wlan1-va2"
#define RTL_DRV_WLAN_P1_VA3_NAME "wlan1-va3"
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
#define RTL_DRV_LAN7_P7_VNETIF_NAME "eth7"
#endif
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#define RTL_DRV_LAN8_P8_VNETIF_NAME "peth0"
#endif

#define RTL_DRV_LAN_P0_NAME "lan0"
#define RTL_DRV_LAN_P1_NAME "lan1"
#define RTL_DRV_LAN_P2_NAME "lan2"
#define RTL_DRV_LAN_P3_NAME "lan3"
#define RTL_DRV_WAN_P4_NAME "wan"
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
#define RTL_DRV_LAN_P7_NAME "lan7"
#endif
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#define RTL_DRV_LAN_P8_NAME "lan8"
#endif

#define RTL_UNTAG		100
#define RTL_INVALIDVLANID 			-100
#define RTL_VLANALREADYEXISTS 	-200
#define RTL_VLANALREADYDEL 		-300
#define RTL_ERRVLANINGRESSDECISION 			-400
#define RTL_ERRVLANINGRESSFILTER 			-500
#define RTL_ERRVLANEGRESSDECISION 			-600
#define RTL_ERRVLANEGRESSFILTER 				-700
#define RTL_ERRVLANEGRESSINSERTVLANTAG 				-800

#define RTL_LAN_DEFAULT_PVID		9
#define RTL_WAN_DEFAULT_PVID		8

#if (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH)) || defined(CONFIG_RTL_8881AM)
#define		RTL_LANPORT_MASK_1		0x1		//port 0
#define		RTL_LANPORT_MASK_2		0x4		//port 1
#define 	RTL_LANPORT_MASK_3		0x8		//port 2
#define 	RTL_LANPORT_MASK_4		0x10	//port 3
#else
#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
#define 	RTL_LANPORT_MASK_1		0x10 	//port 4
#define 	RTL_LANPORT_MASK_2		0x8 	//port 3
#define 	RTL_LANPORT_MASK_3		0x4 	//port 2
#define 	RTL_LANPORT_MASK_4		0x2 	//port 1
#else
#define 	RTL_LANPORT_MASK_1		0x1		//port 0
#define	RTL_LANPORT_MASK_2		0x2		//port 1
#define 	RTL_LANPORT_MASK_3		0x4		//port 2
#define 	RTL_LANPORT_MASK_4		0x8		//port 3
#endif
#endif
#define   IS_FORWARD_PKT_MAGIC_NUM	0xaabbccdd

#ifdef __ECOS
#define TAG_TABLE_FOR_NATD_LIST_MAX	64
#define TAG_TABLE_FOR_NATD_ENTRY_MAX	128
#endif

#ifndef VLAN_HLEN
#define VLAN_HLEN 4
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#ifndef ETH_P_8021Q
#define ETH_P_8021Q     0x8100          /* 802.1Q VLAN Extended Header  */
#endif

#ifndef FAILED
#define FAILED (-1)
#endif

#ifndef SUCCESS
#define SUCCESS (0)
#endif

#ifndef htons
#define htons(x) (x)
#endif

#ifndef ntohs
#define ntohs(x) (x)
#endif

#define	PORT_BASED_VLAN_MODE			(0x1<<0)
#define	MAC_BASED_VLAN_MODE				(0x1<<1)
#define	SUBNET_BASED_VLAN_MODE			(0x1<<2)
#define	PROTOCOL_BASED_VLAN_MODE		(0x1<<3)

typedef enum{
	KEEP_ORI_VLAN_MODE,
	REPALCE_ORI_VLAN_MODE,
}wan_vlan_tag_mode;

enum{
	LAN_WAN_NAPT = 1,
	LAN_WAN_BRIDGE,
};

typedef struct port_mapping_interface_s{
	unsigned char name[IFNAMSIZ];
	unsigned int portMask;
	unsigned short pvid;
	unsigned char cfi;
	unsigned char priority;
}port_mapping_interface;

typedef struct rtl_vlan_entry_s{
	unsigned int memPortMask;	/*port <-> interface*/
	unsigned int untagPortMask;	
	unsigned int valid:1,
			    forward_rule:2,	/*lan <-> wan forward rule*/
			    vid:12;
}rtl_vlan_entry_t;

struct _vlan_tag {
	unsigned short tpid;	// protocol id
	unsigned short pci;		// priority:3, cfi:1, id:12
};

struct vlan_tag {
	union
	{
		unsigned long v;
		struct _vlan_tag f;
	};
};

extern int rtl_addRtlVlanEntry(unsigned short vid);
extern int rtl_delRtlVlanEntry(unsigned short vid);
extern int rtl_addRtlVlanMemPortByDevName(unsigned short vid, unsigned char *name);
extern int rtl_delRtlVlanMemPortByDevName(unsigned short vid, unsigned char *name);
extern int rtl_setRtlVlanPortTagByDevName(unsigned short vid, unsigned char *name, int tag);
extern int rtl_initRtlVlan(void);
#if defined(__ECOS)&&!defined(CONFIG_RTL_NAPT_KERNEL_MODE_SUPPORT)
struct ip_info
{
	unsigned short dir;
	unsigned short protocol;
	unsigned short id;
	unsigned long sip;
	unsigned long dip;
	unsigned long now;
};
int rtl_initTagForNatdTable(int tag_tbl_for_natd_list_max, int tag_tbl_for_natd_entry_max);
int rtl_addTagForNatdEntry(struct ip_info *ipInfo, struct vlan_tag tag);
int rtl_lookupTagForNatdEntry(struct ip_info *ipInfo, struct vlan_tag *tag);
int rtl_delTagForNatdEntry(void);
#endif
#endif  //__RTL_VLAN_H
