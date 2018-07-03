/*
 *	A Linux Ethernet driver for the RealTek 865x chips
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *		All rights reserved.
 *
 */

#ifdef __KERNEL__
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <net/pkt_sched.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <asm/rtl865x/interrupt.h>
#elif defined(__ECOS)
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
//#include <cyg/io/eth/rltk/819x/wrapper/interrupt.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include <sys/mbuf.h>
//#include <netinet/ip6.h>
//#include <sdtlib.h>
#else
#error "Invalid configuration!\n"
#endif

#include "asicregs.h"
#include "rtl_types.h"  
#include "swCore.h"
#include "swNic_poll.h"
#include "vlanTable.h"
#include "rtl8651_layer2.h"
#include "rtl865xC_tblAsicDrv.h"
#ifdef CONFIG_RTL865X_HW_TABLES
#include "tblDrv/rtl865x_lightrome.h"
#endif

#include <switch/rtl865x_fdb_api.h>
#include <switch/rtl865x_arp_api.h>
#include <switch/rtl865x_netif.h>
#include "l3Driver/rtl865x_ip.h"

/*
 * @ Source Code, Ported by zhuhuan @
 * Function name: arpioctl
 * Description: The following code is used for traffic control with the fastnat of realtek
 * Date: 2016.03.01
 */
// +++++
#ifndef CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#define CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#endif

#define PROTO_TCP 6
#define PROTO_UDP 17
#define PROTO_GRE 47
#ifdef __CONFIG_TC__
typedef	struct	ip	ip_t;
#define	NAT_OUTBOUND 0
#define	NAT_INBOUND	 1
extern int stream_control(int nat_dir, int nflags, ip_t *ip/*, fr_info_t *fin*/);
extern  int g_user_rate_control;
#ifdef __CONFIG_TC_WINDOWS__
extern int up_win_stream_control(int nat_dir, ip_t *ip);
extern int down_win_stream_control(int nat_dir, ip_t *ip,int type);
#endif	/*__CONFIG_TC_WINDOWS__*/

#ifdef __CONFIG_STREAM_STATISTIC__
extern int stream_statistic(ip_t * ip, int direction,int type);
extern int g_user_stream_statistic;
#endif/*__CONFIG_STREAM_STATISTIC__*/
#endif/*__CONFIG_TC__*/

#ifndef nvram_safe_get
#define nvram_safe_get(name) (nvram_get(name) ? : "")
#endif
// +++++

#ifdef CONFIG_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
static int block_link_change=0;
#define RTK_BLOCK_LINK_CHANGE_PERIOD 1
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
typedef struct  rtk_int_8367r_status_s
{
    unsigned char value[2];
} rtk_int_8367r_status_t;
#endif

#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
#define RTL_RX_DIR 1
#define RTL_TX_DIR 2

#define GATEWAY_MODE 0
#define WISP_MODE 2
#define CLIENT_MODE 1

static unsigned char wan_if_name[16] = {"eth1"}; //default assume as gateway mode
extern unsigned int	br0_ip_addr;
extern unsigned int	br0_ip_mask;
extern unsigned int	guest_ip_addr;
extern unsigned int	guest_ip_mask;

int rtl_qos_rate_limite_check(int dir, unsigned char *dev_name, struct sk_buff *skb);
#endif

#ifdef CONFIG_RTL_VLAN_SUPPORT
#include <netinet/rtl_vlan.h>
int rtl_vlan_support_enable = 0;
#if defined(CONFIG_RTL_8367R_SUPPORT)
#define CONFIG_RTK_REFINE_PORT_DUPLEX_MODE 1
extern int rtl865x_enableRtl8367ToCpuAcl(void);
extern int rtl865x_disableRtl8367ToCpuAcl(void);
extern int RTL8367R_vlan_set(int mode);
#if  defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
extern int rtk_refinePortDuplexMode(void);
#endif
#endif
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
extern int  rtl_vlanIngressProcess(struct sk_buff *skb, unsigned char *dev_name, struct sk_buff **new_skb);
extern int rtl_vlanEgressProcess(struct sk_buff *skb, unsigned char *dev_name,  int wlan_pri);
#else
extern int rtl_vlanIngressProcess(struct sk_buff *skb, unsigned char *dev_name);
extern int rtl_vlanEgressProcess(struct sk_buff *skb, unsigned char *dev_name,  int wlan_pri);
#endif
#if defined(CONFIG_RTL_HARDWARE_NAT)
extern int flush_hw_table_flag;
#endif
#endif

#ifndef VLAN_HLEN
#define VLAN_HLEN 4
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#if (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH))|| defined(CONFIG_RTL_8881AM)
  #define	RTL_LANPORT_MASK	0x11d
  #define	RTL_WANPORT_MASK	0x2
#elif defined(CONFIG_POCKET_ROUTER_SUPPORT)
  #define	RTL_LANPORT_MASK	0xf
  #define	RTL_WANPORT_MASK	0x10
#else
#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
	//change wan port to port0
   #define RTL_WANPORT_MASK		0x01 //port0
   #define RTL_LANPORT_MASK		0x11e
#else
  #define	RTL_LANPORT_MASK	0x10f
  #define	RTL_WANPORT_MASK	0x10
 #endif
#endif

/* BSPLJF++, 2016-10-31,define WAN port in userSpace/.config*/
#if defined(__CONFIG_WAN_PORT__)
#define CONFIG_TENDA_WAN_PORT __CONFIG_WAN_PORT__
#endif

/* add by jack,,2016-1-12,define LAN & WAN port */
#ifndef CONFIG_TENDA_WAN_PORT
#ifdef __CONFIG_A9__
#define CONFIG_TENDA_WAN_PORT 4
#else
#define CONFIG_TENDA_WAN_PORT 0
#endif
#endif

#undef  RTL_LANPORT_MASK
#undef  RTL_WANPORT_MASK
#undef  RTL_LANPORT_MASK_1
#undef  RTL_LANPORT_MASK_2
#undef  RTL_LANPORT_MASK_3
#undef  RTL_LANPORT_MASK_4

#if CONFIG_TENDA_WAN_PORT == 0
#define     RTL_LANPORT_MASK        0x11e
#define     RTL_WANPORT_MASK        0x1
#define     RTL_LANPORT_MASK_1      0x2     //port 1
#define     RTL_LANPORT_MASK_2      0x4     //port 2
#define     RTL_LANPORT_MASK_3      0x8     //port 3
#define     RTL_LANPORT_MASK_4      0x10    //port 4
//lq 适配port3为wan
#elif CONFIG_TENDA_WAN_PORT == 3

#define     RTL_LANPORT_MASK         0x117
#define     RTL_WANPORT_MASK        0x8      //port3
#define     RTL_LANPORT_MASK_1      0x1     //port0
#define     RTL_LANPORT_MASK_2      0x2     //port 1
#define     RTL_LANPORT_MASK_3      0x4     //port 2
#define     RTL_LANPORT_MASK_4      0x10   //port 4

#elif CONFIG_TENDA_WAN_PORT == 4
#define     RTL_LANPORT_MASK        0x10f
#define     RTL_WANPORT_MASK        0x10
#define     RTL_LANPORT_MASK_1      0x1     //port0
#define     RTL_LANPORT_MASK_2      0x2     //port 1
#define     RTL_LANPORT_MASK_3      0x4     //port 2
#define     RTL_LANPORT_MASK_4      0x8     //port 3

#else
#undef      CONFIG_TENDA_WAN_PORT
#define     CONFIG_TENDA_WAN_PORT   0
#define     RTL_LANPORT_MASK        0x10f
#define     RTL_WANPORT_MASK        0x10
#define     RTL_LANPORT_MASK_1      0x2     //port 1
#define     RTL_LANPORT_MASK_2      0x4     //port 2
#define     RTL_LANPORT_MASK_3      0x8     //port 3
#define     RTL_LANPORT_MASK_4      0x10    //port 4
#endif
/* end add by jack,2016-1-12 */

#ifdef CONFIG_RTL_8881A_CUTE_MAHJONG_ESD
int _cute_mahjong_esd_counter = 0;
int _cute_mahjong_esd_reboot_counter = 0;
extern void rtl_CuteMahjongEsdTimerHandle(void);
#endif

#if defined(CONFIG_RTL_CONE_NAT_SUPPORT)
extern int rtl_initRtlConeNat(void);
#endif

#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
extern int rtl_initRtlPortFwd(void);
#endif

#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
extern int rtl_initTriggerPort(void);
#endif

#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
#include "rtl865xC_tblAsicDrv.h"
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#define CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE 1

int32 rtl_isEthPassthruFrame(uint8 *data);
#endif

#if	defined(CONFIG_RTL_HARDWARE_NAT)
#ifndef PKTHDR_EXTPORT_MAGIC
#define	PKTHDR_EXTPORT_MAGIC		0xA530
#endif
#define	PKTHDR_EXTPORT_MAGIC2		0xA531
#define	PKTHDR_EXTPORT_P1               6
#define	PKTHDR_EXTPORT_P2               7
#define	PKTHDR_EXTPORT_P3               8
#endif

#ifdef CONFIG_RTK_VOIP
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
	#define RX_TASKLET
	#define TX_TASKLET
	#ifdef CONFIG_RTL_8190
		#define BR_SHORTCUT
	#endif
#else //!CONFIG_RTK_VOIP

	#define TX_TASKLET
	#ifdef CONFIG_NET_RADIO
		#define BR_SHORTCUT
	#endif
#if defined(CONFIG_RTL8197B_PANA) || defined(CONFIG_RTL865X_PANAHOST)
	#define RX_TASKLET
#else
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	#define RX_TASKLET
#endif
#endif

#ifndef __ECOS
	#define RTK_QUE
#endif
	#define TIME_STAMP  ( jiffies*10000  +   (REG32(TC0CNT)>>8) *5 ) //jiffies every 10 ms count 1
														            //TC0CNT every 5 us count 1
#endif


#ifdef __ECOS
	#define RX_TASKLET
	#define TX_TASKLET
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_BRSC
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	#define BR_SHORTCUT
#endif
#endif
	#define dev_kfree_skb_any(skb)	kfree_skb_chk_key(skb, reNet[0]);	
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_DRVETH
#include <asm/mips16_lib.h>
#endif

#ifdef BR_SHORTCUT
#ifndef __ECOS
	#define DYNAMIC_ADJUST_TASKLET
#endif
#endif
	
#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
#include <linux/rtk_vlan.h>
#endif

#include <rtl/rtl865x_eventMgr.h>

#if 0
#ifdef __KERNEL__
#define DEBUG_ERR printk

#elif defined(__ECOS)
#define DEBUG_ERR diag_printf
#endif

#else
#define DEBUG_ERR(format, args...)
#endif

#if defined(CONFIG_RTL8196C_AP_HCM) || defined(CONFIG_RTL_819X)
#define RTL819X_PRIV_IOCTL_ENABLE 1 //mark_add
#endif

#ifdef CONFIG_RTL_8198_NFBI_BOARD
#define CONFIG_819X_PHY_RW 1
#endif

#define IS_BRIDGE_PORT(dev) \
	(dev&& netif_running(dev) &&(dev->info)&&(((Rltk819x_t *)(dev->info))->sc)&& (((Rltk819x_t *)(dev->info))->sc->sc_arpcom.ac_if.if_bridge))
#ifdef CONFIG_819X_PHY_RW
#include "rtl865xC_tblAsicDrv.h"

struct port_mibStatistics  {

	/*here is in counters  definition*/
	uint32 ifInOctets;
	uint64 ifHCInOctets;
	uint32 ifInUcastPkts;
	uint64 ifHCInUcastPkts;
	uint32 ifInMulticastPkts;
	uint64 ifHCInMulticastPkts;
	uint64 ifHCInBroadcastPkts;
	uint32 ifInBroadcastPkts;
	uint32 ifInDiscards;
	uint32 ifInErrors;
	uint64 etherStatsOctets;
	uint32 etherStatsUndersizePkts;
	uint32 etherStatsFraments;
	uint32 etherStatsPkts64Octets;
	uint32 etherStatsPkts65to127Octets;
	uint32 etherStatsPkts128to255Octets;
	uint32 etherStatsPkts256to511Octets;
	uint32 etherStatsPkts512to1023Octets;
	uint32 etherStatsPkts1024to1518Octets;
	uint32 etherStatsOversizePkts;
	uint32 etherStatsJabbers;
	uint32 dot1dTpPortInDiscards;
	uint32 etherStatusDropEvents;
	uint32 dot3FCSErrors;
	uint32 dot3StatsSymbolErrors;
	uint32 dot3ControlInUnknownOpcodes;
	uint32 dot3InPauseFrames;

	/*here is out counters  definition*/
	uint32 ifOutOctets;
	uint64 ifHCOutOctets;
	uint32 ifOutUcastPkts;
	uint64 ifHCOutUcastPkts;
	uint64 ifHCOutMulticastPkts;
	uint64 ifHCOutBroadcastPkts;
	uint32 ifOutMulticastPkts;
	uint32 ifOutBroadcastPkts;
	uint32 ifOutDiscards;
	uint32 ifOutErrors;
	uint32 dot3StatsSingleCollisionFrames;
	uint32 dot3StatsMultipleCollisionFrames;
	uint32 dot3StatsDefferedTransmissions;
	uint32 dot3StatsLateCollisions;
	uint32 dot3StatsExcessiveCollisions;
	uint32 dot3OutPauseFrames;
	uint32 dot1dBasePortDelayExceededDiscards;
	uint32 etherStatsCollisions;

	/*here is whole system couters definition*/
	uint32 dot1dTpLearnedEntryDiscards;
	uint32 etherStatsCpuEventPkts;
	uint32 ifInUnknownProtos;	 //mark_rm
	uint32 ifSpeed;
	uint32 ifHighSpeed;
	uint32 ifConnectorPresent;
	uint32 ifCounterDiscontinuityTime;
};
#endif

#ifdef RTL819X_PRIV_IOCTL_ENABLE

#define RTL819X_IOCTL_READ_PORT_STATUS			(SIOCDEVPRIVATE + 0x01)	
#define RTL819X_IOCTL_READ_PORT_STATS	              (SIOCDEVPRIVATE + 0x02)	

struct lan_port_status {
    unsigned char link;
    unsigned char speed;
    unsigned char duplex;
    unsigned char nway;    	
}; 

struct port_statistics  {
	unsigned int  rx_bytes;		
 	unsigned int  rx_unipkts;		
       unsigned int  rx_mulpkts;			
	unsigned int  rx_bropkts;		
 	unsigned int  rx_discard;		
       unsigned int  rx_error;			
	unsigned int  tx_bytes;		
 	unsigned int  tx_unipkts;		
       unsigned int  tx_mulpkts;			
	unsigned int  tx_bropkts;		
 	unsigned int  tx_discard;		
       unsigned int  tx_error;			   
};
#endif


//---------------------------------------------------------------------------
#define DRV_NAME			"Realtek 865x"
#define DRV_VERSION		"0.6"
#define DRV_RELDATE     "Mar 26, 2008"

#ifdef CONFIG_RTL8196_RTL8366
/* prevent boardcast packet rebound ;PlusWang 0429 */
#define PREVENT_BCAST_REBOUND
#endif

#ifdef PREVENT_BCAST_REBOUND
#define UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))
#define P_BCAST_NUM 	5
static int PBR_index ;
struct bcast_entry_s{
	unsigned long time_stamp;
	unsigned char BCAST_SA[6];
};
struct bcast_tr_s{
	struct bcast_entry_s entry[P_BCAST_NUM];
};

struct bcast_tr_s bcast;
#endif
//#define RX_OFFSET 2


#ifndef __ECOS
#ifdef CONFIG_RTL8197B_PANA
#define NUM_RX_PKTHDR_DESC	256
#define NUM_TX_PKTHDR_DESC	512
#else
#define NUM_RX_PKTHDR_DESC	512
#define NUM_TX_PKTHDR_DESC	1024
#endif

#if defined(CONFIG_RTL8196B_GW_8M) 
	#undef NUM_RX_PKTHDR_DESC
	#undef NUM_TX_PKTHDR_DESC
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	#define NUM_RX_PKTHDR_DESC	220
#else
	#define NUM_RX_PKTHDR_DESC	ETH_REFILL_THRESHOLD+16
#endif
	#define NUM_TX_PKTHDR_DESC	256
#endif

#if (defined(CONFIG_RTL8196C_AP_ROOT) && !defined(CONFIG_RTL_92D_SUPPORT)) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
	#undef NUM_RX_PKTHDR_DESC
	#undef NUM_TX_PKTHDR_DESC
	#define NUM_RX_PKTHDR_DESC	256
	#define NUM_TX_PKTHDR_DESC	256
#endif

#if defined(CONFIG_RTL8198_AP_ROOT)
	#undef NUM_RX_PKTHDR_DESC
	#undef NUM_TX_PKTHDR_DESC
	#define NUM_RX_PKTHDR_DESC	512
	#define NUM_TX_PKTHDR_DESC	512
#endif
#endif

//#define MBUF_LEN	1600
//#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)

#define NEXT_DEV(cp)			(cp->dev_next ? cp->dev_next : cp->dev_prev)
#define NEXT_CP(cp)			((struct re_private *)NEXT_DEV(cp)->priv)
#define IS_FIRST_DEV(cp)	(NEXT_CP(cp)->opened ? 0 : 1)
#define GET_IRQ_OWNER(cp) (cp->irq_owner ? cp->dev : NEXT_DEV(cp))
#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETWANLINKSPEED 	2100
#define RTL8651_IOCTL_SETWANLINKSPEED 	2101

#define RTL8651_IOCTL_GETLANLINKSTATUS 2102
#define RTL8651_IOCTL_GETLANLINKSPEED    2103
#define RTL8651_IOCTL_GETETHERLINKDUPLEX 2104

#define RTL8651_IOCTL_GET_ETHER_EEE_STATE 2105
#define RTL8651_IOCTL_GET_ETHER_BYTES_COUNT 2106
#define RTL8651_IOCTL_SET_IPV6_WAN_PORT 2107
#define RTL8651_IOCTL_SETLANLINKSPEED 	2108
#define RTL8651_IOCTL_SETETHERLINKDUPLEX 2109

//llm add
#define TENDA_RTL8651_IOCTL_SETWANLINKSPEED 2110

/*
#ifdef CONFIG_RTK_MESH
#define RTL8651_IOCTL_GETLANLINKSTATUSALL 2105
#endif
*/
	

//---------------------------------------------------------------------------
struct re_private {
	u32			id;            /* VLAN id, not vlan index */
	u32			portmask;     /* member port mask */
	u32			portnum;     	/* number of member ports */
	u32			netinit;
    struct net_device   *dev;
    struct net_device   *dev_prev;
    struct net_device   *dev_next;		
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	struct	net_device	*pDev;
#endif

#ifdef __KERNEL__
#ifdef RX_TASKLET
    struct tasklet_struct   rx_dsr_tasklet;
#endif

#ifdef TX_TASKLET
    struct tasklet_struct   tx_dsr_tasklet;
#endif

    spinlock_t			lock;
#endif
    struct net_device_stats net_stats;

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(BR_SHORTCUT) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)  || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL8196C_KLD)|| defined(CONFIG_RTL8198) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)  || defined(CONFIG_RTL_8197F)
    struct timer_list expire_timer; 
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
    struct timer_list expire_timer2; 
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
    unsigned int    tx_avarage;
    unsigned int    tx_peak;
    unsigned int    rx_avarage;
    unsigned int    rx_peak;
    unsigned int    tx_byte_cnt;
    unsigned int    rx_byte_cnt;    
#endif
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
    unsigned int    tx_avarage;
    unsigned int    tx_peak;
    unsigned int    rx_avarage;
    unsigned int    rx_peak;
    unsigned int    tx_byte_cnt;
    unsigned int    rx_byte_cnt;    
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	struct vlan_info	vlan_setting;
#endif

    unsigned int vid; //vlan id
    unsigned int port; //member port
    int opened; 
    int  irq_owner; //record which dev request IRQ

#ifdef __ECOS
	unsigned int isr_status;
#endif
#if defined(CONFIG_RTL_819X)	//jwj:20120821
	unsigned char is_wan;
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	unsigned char isPdev;
#endif

};

#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG
static int hangup_reinit = 0;
#ifndef __ECOS
extern int skip_free_skb_check;
#endif
#endif
#endif

struct  init_vlan_setting {
	unsigned short vid;
	unsigned int portmask;
	unsigned char mac[6];
	unsigned char is_wan;
	unsigned char is_slave;
};

#ifdef BR_SHORTCUT
__DRAM_SECTION_  unsigned char cached_eth_addr[6];
__DRAM_SECTION_  struct net_device *cached_dev=NULL;

__DRAM_SECTION_  unsigned char cached_eth_addr2[6];
__DRAM_SECTION_  struct net_device *cached_dev2=NULL;

__DRAM_SECTION_  unsigned char cached_eth_addr3[6];
__DRAM_SECTION_  struct net_device *cached_dev3=NULL;

__DRAM_SECTION_  unsigned char cached_eth_addr4[6];
__DRAM_SECTION_  struct net_device *cached_dev4=NULL;
int last_used = 1;
#endif
#ifdef CONFIG_WIRELESS_LAN_MODULE
 EXPORT_SYMBOL(cached_eth_addr);
 EXPORT_SYMBOL(cached_dev);

 
 EXPORT_SYMBOL(cached_eth_addr2);
 EXPORT_SYMBOL(cached_dev2);
 
 EXPORT_SYMBOL(cached_eth_addr3);
 EXPORT_SYMBOL(cached_dev3);
 
 EXPORT_SYMBOL(cached_eth_addr4);
 EXPORT_SYMBOL(cached_dev4);
 #ifdef PERF_DUMP
 int32 (*Fn_rtl8651_romeperfEnterPoint)( uint32 index )=NULL;
  int32  (*Fn_rtl8651_romeperfExitPoint)(uint32 index)=NULL;
 #endif
#endif

#if defined(CONFIG_RTL_8197F)
uint32 sw_pvid[RTL8651_PORT_NUMBER] = {0};
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
//#include "rtl865x_igmpsnooping.h"
#include <rtl/rtl865x_igmpsnooping.h>
#define RTL8651_MAC_NUMBER				6
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <rtl/rtl865x_multicast.h>
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <netinet/ip6.h>
int mldSnoopEnabled=1;
#endif
uint32 nicIgmpModuleIndex=0xFFFFFFFF;

int  igmpsnoopenabled=1;
uint32 brIgmpModuleIndex=0xFFFFFFFF;

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
extern struct net_bridge *bridge0;
extern uint32 br0SwFwdPortMask;
#endif
extern int32 vlanPortmask_get(uint32 vid);

#endif

#define CONFIG_RTK_CHECK_ETH_WAN_PORT_RX_HANG	1

#ifdef CONFIG_RTK_CHECK_ETH_WAN_PORT_RX_HANG	
static int eth_port_rx_hang_count[MAX_PORT_NUMBER]={0};
#endif
static unsigned int AN1tocbcount[MAX_PORT_NUMBER]={0};
static unsigned int AN2tosnrcount[MAX_PORT_NUMBER]={0};
static unsigned int AN3tosnrcount[MAX_PORT_NUMBER]={0};
static int wanNetifMtu = 0;
//---------------------------------------------------------------------------
static char version[] __devinitdata = \
KERN_INFO DRV_NAME " Ethernet driver v" DRV_VERSION " (" DRV_RELDATE ")\n";

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)|| defined(CONFIG_RTL_VLAN_SUPPORT)
#define ETH_INTF_NUM	5
static struct net_device *reNet[ETH_INTF_NUM+3] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
int L2_table_disabled = 0;

#else
#define ETH_INTF_NUM	2
static struct net_device *reNet[ETH_INTF_NUM+3] = {NULL, NULL, NULL, NULL, NULL};
#endif
static struct net_device *pDev = NULL;//for passthrough

#define TASKLET_MASK	(BIT(0)|BIT(1))

#ifdef DYNAMIC_ADJUST_TASKLET
static int rx_tasklet_enabled = 0;			
static int rx_pkt_thres=0;
static int rx_cnt;
#endif

#if 1 //def BR_SHORTCUT
static int eth_flag=12; // bit01: tasklet control,  0 dynamic tasklet, 1 - disable tasklet, 2 - always tasklet , 
									// bit2 - bridge shortcut control (1: enable, 0: disable)
									// bit3 - enable IP multicast route (1: enable, 0: disable)
static int pkt_cnt=0, enable_brsc=0;
#endif

#ifdef DELAY_REFILL_ETH_RX_BUF
#ifdef ETH_NEW_FC
static int during_close = 0;
#endif

#define MAX_PRE_ALLOC_RX_SKB	64
#else
#define MAX_PRE_ALLOC_RX_SKB	160
#endif

#if defined(CONFIG_RTL8196B_GW_8M)
	#undef MAX_PRE_ALLOC_RX_SKB
	#define MAX_PRE_ALLOC_RX_SKB	48
#endif

#if (defined(CONFIG_RTL8196C_AP_ROOT) && !defined(CONFIG_RTL_92D_SUPPORT)) || defined(CONFIG_RTL8196C_CLIENT_ONLY) || defined(CONFIG_RTL8198_AP_ROOT)
	#undef MAX_PRE_ALLOC_RX_SKB
	#define MAX_PRE_ALLOC_RX_SKB	64
#endif

#ifdef __ECOS
#undef NUM_RX_PKTHDR_DESC
#undef NUM_TX_PKTHDR_DESC
#undef MAX_PRE_ALLOC_RX_SKB
#if defined(CONFIG_RTL_8367R_SUPPORT)
#define NUM_RX_PKTHDR_DESC			256//900
#define NUM_TX_PKTHDR_DESC			256//768	
#elif defined(CONFIG_RTL_8197F)
/*buferL增大队列，提高待机量*/
#define NUM_RX_PKTHDR_DESC			1024//256//512
#define NUM_TX_PKTHDR_DESC			512//256//1024
#else
#if defined(CYGNUM_RAM_SIZE_0x00800000) && defined(CONFIG_RTL_8881A) && !defined(CONFIG_CUTE_MAHJONG)
#define NUM_RX_PKTHDR_DESC	128
#else
#define NUM_RX_PKTHDR_DESC	256
#endif
#define NUM_TX_PKTHDR_DESC	256
#endif
#define MAX_PRE_ALLOC_RX_SKB	0
#endif

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	#ifdef CONFIG_RTL8196B
	
		#ifdef DELAY_REFILL_ETH_RX_BUF
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB + 600)

		#else
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB + 400)
		#endif

	#else
	#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB + 256)
	#endif

	#if defined(CONFIG_RTL8196B_GW_8M) 
		#undef MAX_ETH_SKB_NUM
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB)
#else
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB + 32)
#endif
	#endif	
	
	#if (defined(CONFIG_RTL8196C_AP_ROOT) && !defined(CONFIG_RTL_92D_SUPPORT)) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
		#undef MAX_ETH_SKB_NUM
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB)	
	#endif	
	
	#if defined(CONFIG_RTL8198_AP_ROOT)
		#undef MAX_ETH_SKB_NUM
		#define MAX_ETH_SKB_NUM	(NUM_RX_PKTHDR_DESC + MAX_PRE_ALLOC_RX_SKB + 600)
	#endif	
		
	int eth_skb_free_num=MAX_ETH_SKB_NUM;

extern int reused_skb_num;
static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
static void init_priv_eth_skb_buf(void);
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
#define RTL8367R_WAN			0		// WAN port is set to 8367R port 4
#else
#define RTL8367R_WAN			4		// WAN port is set to 8367R port 4
#endif
rtk_stat_port_cntr_t  port_cntrs;
#endif

#ifdef RTK_QUE
struct ring_que {
	int qlen;
	int qmax;	
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_RX_SKB+1];
};
__DRAM_SECTION_ static struct ring_que rx_skb_queue;

#else

__DRAM_SECTION_ static struct sk_buff_head rx_skb_queue; 
#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
static unsigned int rxRingSize[RTL865X_SWNIC_RXRING_MAX_PKTDESC] = {NUM_RX_PKTHDR_DESC};
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_MAX_PKTDESC] = {NUM_TX_PKTHDR_DESC};
#else
static unsigned int rxRingSize[RTL865X_SWNIC_RXRING_MAX_PKTDESC] = {NUM_RX_PKTHDR_DESC, 0, 0, 0, 0, 0};
static unsigned int txRingSize[RTL865X_SWNIC_TXRING_MAX_PKTDESC] = {NUM_TX_PKTHDR_DESC, 0};
#endif
static struct  init_vlan_setting vlanSetting[] = {
#if defined(__ECOS)
#ifdef CONFIG_POCKET_ROUTER_SUPPORT
    {LAN_VID,  (PORT_HW_AP_MODE|EXT_PORT_HWLOOKUP)  ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}, 0, 0},  // eth0(LAN) VLAN P0-P3
    {WAN_VID,  0,                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}, 0, 0},  // eth1(WAN) VLAN P4
#else
#ifdef CONFIG_RTL_VLAN_SUPPORT
    {LAN_VID,  RTL_LANPORT_MASK_1,  				   {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x01}, 0, 0},  // eth0(LAN) VLAN P0-P3
#else
    {LAN_VID,  (ALL_PORTS & (~RTL_WANPORT_MASK))  ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}, 0, 0},  // eth0(LAN) VLAN P0-P3
#endif
    {WAN_VID,  RTL_WANPORT_MASK ,                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22} ,1, 0},  // eth1(WAN) VLAN P4
#ifdef CONFIG_RTL_VLAN_SUPPORT
    {LAN_VID,  RTL_LANPORT_MASK_2,                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x02} ,0, 0},  // eth2(LAN) for Guest Zone
    {LAN_VID,  RTL_LANPORT_MASK_3,                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x03} ,0, 0},  // eth3(LAN) for Guest Zone
    {LAN_VID,  RTL_LANPORT_MASK_4,                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x04} ,0, 0},  // eth4(LAN) for Guest Zone
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    {LAN_VID,  0,                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x07} ,1, 0},  // eth7(LAN) for bridge vlan
#endif
#endif
#endif    
#elif defined(CONFIG_RTL8197B_PANA)
    {LAN_VID,  (ALL_PORTS & (~BIT(0)))  ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x98},0 , 0},  // eth0(LAN) VLAN P1-P4
    {WAN_VID,  BIT(0) ,                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x97},1 , 0},  // eth1(WAN) VLAN P0 
#elif defined(CONFIG_RTL8196B_GW_MP) || defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY)
    {LAN_VID,  (ALL_PORTS)  ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},0 , 0},  // eth0(LAN) VLAN P1-P4    
    {WAN_VID,  0 ,                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},1 , 0},  // eth1(WAN) VLAN P0 
#else
    {LAN_VID,  (ALL_PORTS & (~BIT(0)))  ,  {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},0 , 0},  // eth0(LAN) VLAN P1-P4
    {WAN_VID,  BIT(0) ,                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},1 , 0},  // eth1(WAN) VLAN P0 
#endif

#if defined(CONFIG_RTK_GUEST_ZONE)
#if defined(CONFIG_RTL8196C_KLD)
    {LAN2_VID,  BIT(2),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x02},0 , 0},  // eth2(LAN) for Guest Zone
    {LAN3_VID,  BIT(1),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x03},0 , 0},  // eth3(LAN) for Guest Zone
    {LAN4_VID,  BIT(0),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x04},0 , 0},  // eth4(LAN) for Guest Zone
#else
    {LAN2_VID,  BIT(2),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x02},0 , 0},  // eth2(LAN) for Guest Zone
    {LAN3_VID,  BIT(3),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x03},0 , 0},  // eth3(LAN) for Guest Zone
    {LAN4_VID,  BIT(4),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x04},0 , 0},  // eth4(LAN) for Guest Zone
#endif    
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
    {LAN2_VID,  BIT(1),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x02},0 , 0},  // eth2(LAN) for Guest Zone
    {LAN3_VID,  BIT(2),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x03},0 , 0},  // eth3(LAN) for Guest Zone
    {LAN4_VID,  BIT(3),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x04},0 , 0},  // eth4(LAN) for Guest Zone
#else
    {LAN2_VID,  BIT(3),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x02},0 , 0},  // eth2(LAN) for Guest Zone
    {LAN3_VID,  BIT(2),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x03},0 , 0},  // eth3(LAN) for Guest Zone
    {LAN4_VID,  BIT(1),                               {0x00, 0xe0, 0x4c, 0xf0, 0x00, 0x04},0 , 0},  // eth4(LAN) for Guest Zone
#endif
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER)
    {WAN_VID,  RTL_WANPORT_MASK,                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},1, 1 },  //ppp0
#endif
};
const uint32 cPVCR[6][4] = {
	{(LAN_VID << 16) | WAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID,},
	{(WAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID,},
	{(LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | WAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID,},
	{(LAN_VID << 16) | LAN_VID, (WAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID,},
	{(LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | WAN_VID, (LAN_VID << 16) | LAN_VID,},
	{(LAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID, (WAN_VID << 16) | LAN_VID, (LAN_VID << 16) | LAN_VID,},
};

#ifdef CONFIG_RTL865X_HW_TABLES
extern struct rtl865x_lrConfig *lrConfig;
#else
void SoftNAT_OP_Mode(int count);
#endif
int savOP_MODE_value=2;

#ifdef CONFIG_RTL8196C_ETH_IOT
uint32 port_link_sts = 0;	// the port which already linked up does not need to check ...
uint32 port_linkpartner_eee = 0;
#endif

//#ifdef CONFIG_RTL_8197D_DYN_THR
#if 1 //defined(CONFIG_RTL_8367R_SUPPORT) ||defined(CONFIG_RTL_8197D_DYN_THR)
unsigned int curLinkPortMask=0;
unsigned int newLinkPortMask=0;
#endif

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
#define MEM_CTRL		0xbb804234
uint32 port_link_sts2 = 0;
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
#if defined(RTK_VLAN_ROUTETYPE_PASS_SRC_INFO)
extern int  rx_vlan_process(struct net_device *dev, struct vlan_info *info_ori, struct sk_buff *skb,  struct sk_buff **new_skb);
#else
extern int  rx_vlan_process(struct net_device *dev, struct vlan_info *info, struct sk_buff *skb);
#endif
extern int  tx_vlan_process(struct net_device *dev, struct vlan_info *info, struct sk_buff *skb, int wlan_pri);
#endif

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
unsigned char wan_if[16] = {0};
#endif //#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)

#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
#include "rtl8651_layer2.h"

#define MIB_RX_PKT_CNT	0
#define MIB_TX_PKT_CNT	1

#ifdef CONFIG_RTL_8197F
#define PORT_GPIO_BASE		22	// Base of P0~P4 at PEFGH, port 0~4: G6~H2
#define PGPIO_CNR 			PEFGH_CNR
#define PGPIO_DIR 			PEFGH_DIR
#define PGPIO_DAT 			PEFGH_DAT
#else
#define PORT_PABCD_BASE	10	// Base of P0~P1 at PABCD
#define P0_PABCD_BIT		10
#define P1_PABCD_BIT		11
#define P2_PABCD_BIT		12
#define P3_PABCD_BIT		13
#define P4_PABCD_BIT		14
#define PORT_GPIO_BASE		PORT_PABCD_BASE
#define PGPIO_CNR 			PABCD_CNR
#define PGPIO_DIR 			PABCD_DIR
#define PGPIO_DAT 			PABCD_DAT
#endif

#define SUCCESS 0
#define FAILED -1

//#define GPIO_LED_MAX_PACKET_CNT	5000
//#define GPIO_LED_MAX_SCALE			100
#define GPIO_LED_NOBLINK_TIME		120	// time more than 1-sec timer interval
//#define GPIO_LED_INTERVAL_TIME		50	// 500ms
#define GPIO_LED_ON_TIME			4	// 40ms
#define GPIO_LED_ON					0
#define GPIO_LED_OFF				1
#define GPIO_LINK_STATUS			1
#define GPIO_LINK_STATE_CHANGE 0x80000000

#define GPIO_UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

struct ctrl_led {
	struct timer_list	LED_Timer;	
	unsigned int		LED_Interval;
	unsigned int		LED_tx_cnt_log;
	unsigned int		LED_rx_cnt_log;
	unsigned int		LED_tx_cnt;
	unsigned int		LED_rx_cnt;
	unsigned int		link_status;
	unsigned char		LED_Toggle;
	unsigned char		LED_ToggleStart;
	unsigned char		blink_once_done;	// 1: blink once done
} led_cb[5];

static void gpio_set_led(int port, int flag){	
	if (flag == GPIO_LED_OFF){
//		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
//		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
		WRITE_MEM32(PABCD_DAT, (READ_MEM32(PABCD_DAT) | ((0x1<<port)<<PORT_PABCD_BASE)));//set 1
	}
	else{
//		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~((0x1<<port)<<PORT_PABCD_BASE)));	//set GPIO pin
//		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x1<<port)<<PORT_PABCD_BASE));//output pin
		WRITE_MEM32(PABCD_DAT, (READ_MEM32(PABCD_DAT) & (~((0x1<<port)<<PORT_PABCD_BASE))));//set 0
	}
}

static void gpio_led_Interval_timeout(unsigned long port)
{
	struct ctrl_led *cb	= &led_cb[port];
	unsigned long flags;

	save_and_cli(flags);

	if (cb->link_status & GPIO_LINK_STATE_CHANGE) {
		cb->link_status &= ~GPIO_LINK_STATE_CHANGE;	
		if (cb->link_status & GPIO_LINK_STATUS)	 
			cb->LED_ToggleStart = GPIO_LED_ON;
		else
			cb->LED_ToggleStart = GPIO_LED_OFF;
		cb->LED_Toggle = cb->LED_ToggleStart;
		gpio_set_led(port, cb->LED_Toggle);			
	}
	else {
		if (cb->link_status & GPIO_LINK_STATUS)	 
			gpio_set_led(port, cb->LED_Toggle);	
	}

	if (cb->link_status & GPIO_LINK_STATUS)	 {
		if (cb->LED_Toggle == cb->LED_ToggleStart) {
			mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);
			cb->blink_once_done=1;
		}
		else {
			mod_timer(&cb->LED_Timer, jiffies + GPIO_LED_ON_TIME);
			cb->blink_once_done=0;
		}
		//cb->LED_Toggle = (cb->LED_Toggle + 1) % 2;
		cb->LED_Toggle = (cb->LED_Toggle + 1) & 0x1;
	}
	restore_flags(flags);
}

void calculate_led_interval(int port)
{
	struct ctrl_led *cb = &led_cb[port];

	unsigned int delta = 0;
	//int i, scale_num=0;

	// calculate counter delta
	delta += GPIO_UINT32_DIFF(cb->LED_tx_cnt, cb->LED_tx_cnt_log);
	delta += GPIO_UINT32_DIFF(cb->LED_rx_cnt, cb->LED_rx_cnt_log);
	cb->LED_tx_cnt_log = cb->LED_tx_cnt;
	cb->LED_rx_cnt_log = cb->LED_rx_cnt;

	// update interval according to delta
	if (delta == 0) {
		if (cb->LED_Interval == GPIO_LED_NOBLINK_TIME)
			mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
		else{
			cb->LED_Interval = GPIO_LED_NOBLINK_TIME;
			if(cb->blink_once_done==1){
				mod_timer(&(cb->LED_Timer), jiffies + cb->LED_Interval);
				cb->blink_once_done=0;
			}
		}
	}
	else
	{	
		if(delta>25){		//That is: 200/delta-GPIO_LED_ON_TIME < GPIO_LED_ON_TIME
			cb->LED_Interval = GPIO_LED_ON_TIME;
		}
		else{
			//if delta is odd, should be +1 into even.
			//just make led blink more stable and smooth.
			if((delta & 0x1) == 1)
				delta++;
			
			cb->LED_Interval=200/delta-GPIO_LED_ON_TIME;		// rx 1pkt + tx 1pkt => blink one time!

//			if (cb->LED_Interval < GPIO_LED_ON_TIME)
//				cb->LED_Interval = GPIO_LED_ON_TIME;
		}
	}
}

void update_mib_counter(int port)
{
	uint32 regVal;
	uint32 addrOffset_fromP0 =0;	
	struct ctrl_led *cb = &led_cb[port];

	regVal=READ_MEM32(PSRP0+(port<<2));
	if((regVal&PortStatusLinkUp)!=0){
		//link up
		if (!(cb->link_status & GPIO_LINK_STATUS)) {
			cb->link_status = GPIO_LINK_STATE_CHANGE | 1;
		}
		addrOffset_fromP0 = port* MIB_ADDROFFSETBYPORT;

		cb->LED_tx_cnt = READ_MEM32( MIB_COUNTER_BASE + OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) + 
			READ_MEM32( MIB_COUNTER_BASE + OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) + 
			READ_MEM32( MIB_COUNTER_BASE + OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 );

		cb->LED_rx_cnt = READ_MEM32( MIB_COUNTER_BASE + OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) + 
			READ_MEM32( MIB_COUNTER_BASE + OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) + 
			READ_MEM32( MIB_COUNTER_BASE + OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 );
	}
	else{
		//link down
		if (cb->link_status & GPIO_LINK_STATUS) {
			cb->link_status = GPIO_LINK_STATE_CHANGE;
		}
	}
}

void init_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];

	WRITE_MEM32(PGPIO_CNR, READ_MEM32(PGPIO_CNR) & (~((0x1<<port)<<PORT_GPIO_BASE)));	//set GPIO pin
	WRITE_MEM32(PGPIO_DIR, READ_MEM32(PGPIO_DIR) | ((0x1<<port)<<PORT_GPIO_BASE));//output pin
	memset(cb, '\0', sizeof(struct ctrl_led));

	update_mib_counter(port);		
	calculate_led_interval(port);	
	cb->link_status |= GPIO_LINK_STATE_CHANGE;

	init_timer(&cb->LED_Timer);
#ifdef __KERNEL__
	cb->LED_Timer.expires = jiffies + cb->LED_Interval;
#endif
	cb->LED_Timer.data = (unsigned long)port;
	cb->LED_Timer.function = gpio_led_Interval_timeout;	
	mod_timer(&cb->LED_Timer, jiffies + cb->LED_Interval);

	gpio_led_Interval_timeout(port);		
}

void disable_led_ctrl(int port)
{
	struct ctrl_led *cb	= &led_cb[port];
	gpio_set_led(port, GPIO_LED_OFF);

	if (timer_pending(&cb->LED_Timer))	
		del_timer_sync(&cb->LED_Timer);
}
#endif // CONFIG_RTL8196C_ETH_LED_BY_GPIO

#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
extern unsigned int rx_noBuffer_cnt;
extern unsigned int rx_noBuffer_cnt1;
extern unsigned int tx_ringFull_cnt;
#endif


int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd);
static unsigned int totalVlans = sizeof(vlanSetting)/sizeof(struct init_vlan_setting);
/* wan_port will be 0 ~ 5 */
#if defined(__ECOS)
#if defined(CONFIG_RTL_8881A) && defined(CONFIG_RTL_ALP)
unsigned int wan_port = 1;
#else
#if (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH))|| defined(CONFIG_RTL_8881AM)
	unsigned int wan_port = 1;
  #else
	unsigned int wan_port = CONFIG_TENDA_WAN_PORT;
  #endif
#endif
#else
  unsigned int wan_port; 
#endif

unsigned int chip_id; 
unsigned int chip_revision_id; 

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
extern unsigned int    wan_vlan_id_proto;
extern unsigned int    wan_vlan_id_data;
extern unsigned int    wan_vlan_id_video;
#endif

extern int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData);
extern int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData);
extern int32 rtl8651_restartAsicEthernetPHYNway(uint32 port, uint32 phyid);

#ifdef CONFIG_RTK_VOIP_PORT_LINK
struct timer_list link_state_timer;
struct sock *voip_rtnl;
int link_state_timer_work = 0;
#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
extern int eee_enabled;
extern void eee_phy_enable(void);
extern void eee_phy_disable(void);
#ifndef __ECOS
static unsigned char prev_port_sts[MAX_PORT_NUMBER] = { 0, 0, 0, 0, 0 };
#endif
#endif

int bonding_type = 0;

#if defined(CONFIG_RTL_8881A)
#if defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG)
int cmj_board_use_port4 = 0;
#endif
#endif

//---------------------------------------------------------------------------
#if 0
#define PRINT_INFO diag_printf

void eth_debug_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) {
	    PRINT_INFO("%s\n", label);
	}

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++)
    {
        PRINT_INFO("\t");
        for (j=0; j<16; j++)
        {
            PRINT_INFO("%02x ", data[j + (i<<4)]);
        }
        PRINT_INFO("\n");
    }

    if (block_remainder > 0)
    {
        PRINT_INFO("\t");
        for (j=0; j<block_remainder; j++)
        {
            PRINT_INFO("%02x ", data[j+(num_blocks<<4)]);
        }
        PRINT_INFO("\n");
    }
}
#endif

#ifdef RTK_QUE
static void rtk_queue_init(struct ring_que *que)
{
	memset(que, '\0', sizeof(struct ring_que));
	que->qmax = MAX_PRE_ALLOC_RX_SKB;
}

static int rtk_queue_tail(struct ring_que *que, struct sk_buff *skb)
{
	int next;
	uint32 flags;

	save_and_cli(flags);

	if (que->head == que->qmax)
		next = 0;
	else
		next = que->head + 1;
	
	if (que->qlen >= que->qmax || next == que->tail) {
		printk("%s: ring-queue full!\n", __FUNCTION__);
		restore_flags(flags);							
		return 0;
	}	
	
	que->ring[que->head] = skb;
	que->head = next;
	que->qlen++;

	restore_flags(flags);					
	
	return 1;
}

__IRAM_SECTION_
static struct sk_buff *rtk_dequeue(struct ring_que *que)
{
	struct sk_buff *skb;
	uint32 flags;

	save_and_cli(flags);

	if (que->qlen <= 0 || que->tail == que->head) {
		restore_flags(flags);							
		return NULL;
	}

	skb = que->ring[que->tail];
		
	if (que->tail == que->qmax)
		que->tail  = 0;
	else
		que->tail++;

	que->qlen--;	

	restore_flags(flags);					
	
	return (struct sk_buff *)skb;
}
#endif // RTK_QUE

#ifdef CONFIG_RTK_VOIP_PORT_LINK
static int rtnetlink_fill_ifinfo(struct sk_buff *skb, struct net_device *dev,
				 int type, u32 pid, u32 seq, u32 change)
{
	struct ifinfomsg *r;
	struct nlmsghdr  *nlh;
	unsigned char	 *b = skb->tail;

	nlh = NLMSG_PUT(skb, pid, seq, type, sizeof(*r));
	if (pid) nlh->nlmsg_flags |= NLM_F_MULTI;
	r = NLMSG_DATA(nlh);
	r->ifi_family = AF_UNSPEC;
	r->ifi_type = dev->type;
	r->ifi_index = dev->ifindex;
	r->ifi_flags = dev->flags;
	r->ifi_change = change;
	nlh->nlmsg_len = skb->tail - b;
	return skb->len;

nlmsg_failure:
	skb_trim(skb, b - skb->data);
	return -1;
}

static void netlink_sedmsg(unsigned long task_priv){
	struct re_private *cp = (struct re_private *)task_priv;
	struct net_device* dev = cp ->dev;
	struct sk_buff *skb;
	if (voip_rtnl == NULL)
		panic("rtnetlink_init: cannot initialize rtnetlink\n");
	//skb = dev_alloc_skb(sizeof(struct ifinfomsg));
	skb = alloc_skb(sizeof(struct ifinfomsg),GFP_ATOMIC);
	//skb = alloc_skb(sizeof(struct ifinfomsg),GFP_KERNEL);
	if (!skb)
		return;
	if (rtnetlink_fill_ifinfo(skb, dev, RTM_LINKCHANGE, 0, 0, 0) < 0) {
		kfree_skb(skb);
		return;
	}
	NETLINK_CB(skb).dst_groups = RTMGRP_LINK;
	netlink_broadcast(voip_rtnl, skb, 0, RTMGRP_LINK, GFP_KERNEL);
}
static void link_state_timer_action(unsigned long task_priv)
{
	static int LinkState_pre = 0;
	struct re_private *cp = ((struct net_device *)task_priv)->priv;
	int LinkState = 0;
	int tmp = 0;
	int i = 0;
	for( i = 0; i < 5; i ++ ) {
		rtl8651_getAsicEthernetLinkStatus(i , &tmp);
		if( tmp )
			LinkState |= ( 1 << i );
	}
	if(LinkState_pre != LinkState){
		netlink_sedmsg((unsigned long)cp);
	}
	LinkState_pre = LinkState;
   	mod_timer(&link_state_timer, jiffies + 100);
}
#endif

#if	0	//def CONFIG_RTK_PORT_HW_IGMPSNOOPING
/*
  process hw multicast forwarding table ,
  when port is non cable plug in should not exist this one table entry 
  co-work with bridge module ;
  check define flag: "CONFIG_RTK_PORT_HW_IGMPSNOOPING" at linux-2.4.18/net/Config.in
  plusWang 2009-0311
*/
#include "rtl865xC_tblAsicDrv.h"
extern int br_portlist_update(	unsigned int IP , int port ,int sourceRM);

int32 rtl8651_getAsicEthernetLinkStatus(uint32 port, int8 *linkUp) 
{

	if(port >= (RTL8651_PORT_NUMBER+3) || linkUp == NULL)
		return FAILED;
	
#if	1	//def CONFIG_RTK_VOIP
	int status = READ_MEM32( PSRP0 + port * 4 );
	if(status & PortStatusLinkUp)
		*linkUp = TRUE;
	else
		*linkUp = FALSE;
#else
	*linkUp = rtl8651AsicEthernetTable[port].linkUp == 1? TRUE: FALSE;
#endif
	return SUCCESS;
}


//PLUS
void interrupt_link_state_for_multicast(unsigned long task_priv)
{

	static int LinkState_init = 0;
	static int LinkState_pre = 0;
	int LinkState = 0;
	char tmp = 0x0;
	int PortNum;
	int eindex;	//entry index

	//check port1~port4
	for( PortNum = 1; PortNum < 5; PortNum ++ ) {
		rtl8651_getAsicEthernetLinkStatus(PortNum , &tmp);
		//if this port is up then return TURE
		if( tmp )
			LinkState |= ( 1 << PortNum );
	}

	if(LinkState_init == 0 ){
		LinkState_pre = LinkState;
		LinkState_init = 1;
		return;
	}

	/*some port state has changed*/ 
	
	if(LinkState_pre != LinkState){

		
		for( PortNum = 1; PortNum < 5; PortNum ++ ) {

			//just check 1->0 case
			if(( (LinkState & (1<<PortNum)) != (LinkState_pre & (1<<PortNum)) ) && ((LinkState & (1<<PortNum))==0))
			{
	
				rtl865x_tblAsicDrv_multiCastParam_t asic_mc;
				
				for( eindex = 0; eindex < RTL8651_IPMULTICASTTBL_SIZE; eindex++ ) {
					// by eindex 0 to 63 , if can't found go on by next
					if (rtl8651_getAsicIpMulticastTable(eindex,  &asic_mc) == FAILED)
						continue;

					/*when source port is plug-out*/ 
					if (asic_mc.port == PortNum)
					{						
						br_portlist_update(asic_mc.dip ,PortNum ,1);
						//panic_printk("source port be removed ; (port:%d;index=%d)!!\n", PortNum , eindex);						
						//dump_multicast_table();		
						break;
					}

					/*when client port is plug-out*/ 
					if(asic_mc.mbr & (1<<PortNum)){						
						br_portlist_update(asic_mc.dip , PortNum ,0);
						//panic_printk("some client gone; (port:%d;index=%d)!!\n",PortNum,eindex);	
						//dump_multicast_table();	
						break;
					}	
					
				}


			
			}
		}		
	}
	
	LinkState_pre = LinkState;

	
}	
#endif

//---------------------------------------------------------------------------
/* static */

/* 
#ifdef CONFIG_RTK_MESH

extern void br_signal_pathsel(void);

#if 0


#include "rtl8651_layer2.h" 


void interrupt_link_state_for_mesh()
{

	static int LinkState_init = 0;
	static int LinkState_pre = 0;
	int LinkState = 0;
	char tmp = 0x0;
	int PortNum;

	//check port1~port4
	for( PortNum = 1; PortNum < 5; PortNum ++ ) {
		rtl8651_getAsicEthernetLinkStatus(PortNum , &tmp);
		//if this port is up then return TURE
		if( tmp )
			LinkState |= ( 1 << PortNum );
	}

	if(LinkState_init == 0 ){
		LinkState_pre = LinkState;
		LinkState_init = 1;
		return;
	}

	
	if(LinkState_pre != LinkState){
		
		for( PortNum = 1; PortNum < 5; PortNum ++ ) {
			//just check 1->0 case
			if(( (LinkState & (1<<PortNum)) != (LinkState_pre & (1<<PortNum)) ))
			{
				if (LinkState == 0) {
					if (LinkState_pre > 0)
						br_signal_pathsel();
				}
				else {
					if (LinkState_pre == 0)
						br_signal_pathsel();	
				}
			}
		}		
	}
	
	LinkState_pre = LinkState;

	
}
#endif

#endif
*/

#ifndef __ECOS
void refill_rx_skb(void)
{
	struct sk_buff *skb;

	while (rx_skb_queue.qlen < MAX_PRE_ALLOC_RX_SKB) {

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else
		skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
		if (skb == NULL) { 
//			DEBUG_ERR("EthDrv: dev_alloc_skb() failed!\n");		
			return;
		}
		skb_reserve(skb, RX_OFFSET);	

#ifdef DELAY_REFILL_ETH_RX_BUF
		{
		extern int check_refill_eth_rx_ring(struct sk_buff *skb);
		
		if  (check_refill_eth_rx_ring(skb))
			continue;
		}
#endif
		
#ifdef RTK_QUE
		rtk_queue_tail(&rx_skb_queue, skb);
#else		
//		__skb_queue_tail(&rx_skb_queue, skb);
		skb_queue_tail(&rx_skb_queue, skb);
#endif
	}
}
#endif

//---------------------------------------------------------------------------
static void free_rx_skb(void)
{
#ifndef __ECOS
	struct sk_buff *skb;
#endif
	RTL_swNic_freeRxBuf();
#ifndef __ECOS
	while  (rx_skb_queue.qlen > 0) {
#ifdef RTK_QUE
		skb = rtk_dequeue(&rx_skb_queue);
#else
//		skb = __skb_dequeue(&rx_skb_queue);
		skb = skb_dequeue(&rx_skb_queue);
#endif
		dev_kfree_skb_any(skb);
	}
#endif
}

//---------------------------------------------------------------------------
void *UNCACHED_MALLOC(int size) 
{
	unsigned int addr;
	addr=((uint32)kmalloc(size, GFP_KERNEL));
	_dma_cache_wback_inv(addr,size);
	addr |= (UNCACHE_MASK);
	return ((void *)addr);	
}

//---------------------------------------------------------------------------
//__IRAM_SECTION_
//__MIPS16
unsigned char *alloc_rx_buf(void **skb, int buflen)
{
	struct sk_buff *new_skb;

#ifndef __ECOS
#ifdef RTK_QUE
	new_skb = rtk_dequeue(&rx_skb_queue);
#else
	new_skb = skb_dequeue(&rx_skb_queue);
#endif
#endif
#ifndef __ECOS
	if (new_skb == NULL) {
#endif
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		new_skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else        
		new_skb = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
		if (new_skb == NULL) { 
			//DEBUG_ERR("EthDrv: alloc_rx_buf() failed!\n");
			return NULL;
		}
		else
			skb_reserve(new_skb, RX_OFFSET);
#ifndef __ECOS
	}
#endif

#if defined(DELAY_REFILL_ETH_RX_BUF)  && defined (CONFIG_RTL_DELAY_REFILL)
	new_skb->priv=NULL;
#endif

	*skb = new_skb;

#ifndef ETH_NEW_FC
	#ifdef RTL_ETH_RX_RUNOUT
	/* store the skb pointer in a DW in front of  new_skb->data, it will be used in swNic_receive() */
	*(uint32 *)(new_skb->data-6) = (uint32)(new_skb);
	#endif
#endif
	
	return new_skb->data;	
}

//---------------------------------------------------------------------------
void free_rx_buf(void *skb)
{
	dev_kfree_skb_any((struct sk_buff *)skb);
}


#if defined(DELAY_REFILL_ETH_RX_BUF) && defined(CONFIG_RTL_DELAY_REFILL)
int rtk_eth_delay_refill(void *skb)
{
#ifdef DELAY_REFILL_ETH_RX_BUF
			extern int return_to_rx_pkthdr_ring(unsigned char *skb);
#ifdef ETH_NEW_FC
			if (during_close || !return_to_rx_pkthdr_ring(skb)) 
#else
			if (!return_to_rx_pkthdr_ring(skb)) 
#endif
#endif
				return 0;
	return 1;
}
#endif
//---------------------------------------------------------------------------
void tx_done_callback(void *skb)
{
	dev_kfree_skb_any((struct sk_buff *)skb);
}

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
void rtl865x_pptpl2tp_extport_recv(struct sk_buff *skb)
{
	struct ethhdr *eth;
	unsigned char mac[] = { 0x00, 0xe0, 0x4c, 0x00, 0x00, 0x01 };
	
	eth = (struct ethhdr *)skb->data;
	if (!memcmp(eth->h_dest, mac, 6)) {
		skb->cb[0] = 'P'; skb->cb[1] = 'P'; skb->cb[2] = 'P';
	}
	else {
		skb->cb[0] = 'N'; skb->cb[1] = 'A'; skb->cb[2] = 'T';
	}
	skb->protocol = eth_type_trans(skb, skb->dev);
	if (skb->pkt_type == PACKET_OTHERHOST)
		skb->pkt_type = PACKET_HOST;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	netif_rx(skb);			
}

void rtl865x_pptpl2tp_extport_xmit(struct sk_buff *skb)
{
	unsigned int vid, port;
	
	vid = 10;
	port = BIT(7);
    if (RTL_swNic_send((void *)skb, skb->data, skb->len, vid, port) < 0) {   
		dev_kfree_skb_irq(skb);
    }	
}
#endif

//---------------------------------------------------------------------------
#ifdef BR_SHORTCUT
#ifdef CONFIG_WIRELESS_LAN_MODULE
 #ifdef PERF_DUMP
extern  int32 (*Fn_rtl8651_romeperfEnterPoint)( uint32 index );
extern  int32  (*Fn_rtl8651_romeperfExitPoint)(uint32 index);
EXPORT_SYMBOL(Fn_rtl8651_romeperfEnterPoint);
EXPORT_SYMBOL(Fn_rtl8651_romeperfExitPoint);
 #endif
extern struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da);
EXPORT_SYMBOL(wirelessnet_hook_shortcut);
struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da)=NULL;
#endif
#endif
static inline int32 rtl_isWanDev(struct re_private *cp)
{
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	return (!cp->vlan_setting.is_lan);
#else
	#if defined(CONFIG_RTL_MULTIPLE_WAN)
		return (cp->id==RTL_WANVLANID || cp->id == RTL_WAN_1_VLANID);
	#else
		return (cp->is_wan );
	#endif
#endif
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
static inline struct iphdr * re865x_getIpv4Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct iphdr *iph=NULL;

	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	/*it's not ipv4 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
	{
		return NULL;
	}

	iph=(struct iphdr *)(ptr+2);

	return iph;
}

#if defined (CONFIG_RTL_MLD_SNOOPING)
static inline struct ip6_hdr* re865x_getIpv6Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct ip6_hdr *ipv6h=NULL;

	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}

	/*it's not ipv6 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IPV6))
	{
		return NULL;
	}

	ipv6h=(struct ip6_hdr *)(ptr+2);

	return ipv6h;
}
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define  HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define  FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60

#define PIM_PROTOCOL 103
#define MOSPF_PROTOCOL 89
#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58

#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143

#define IS_IPV6_PIM_ADDR(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x0000000D))
#define IS_IPV6_MOSPF_ADDR1(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000005))
#define IS_IPV6_MOSPF_ADDR2(ipv6addr) ((ipv6addr[0] == 0xFF020000)&&(ipv6addr[1] == 0x00000000)&&(ipv6addr[2] == 0x00000000)&&(ipv6addr[3] ==0x00000006))
int re865x_getIpv6TransportProtocol(struct ip6_hdr* ipv6h)
{

	unsigned char *ptr=NULL;
	unsigned char *startPtr=NULL;
	unsigned char *lastPtr=NULL;
	unsigned char nextHeader=0;
	unsigned short extensionHdrLen=0;

	unsigned char  optionDataLen=0;
	unsigned char  optionType=0;
	unsigned int ipv6RAO=0;
	unsigned int ipv6addr[4] = {0};

	if(ipv6h==NULL)
	{
		diag_printf("(ipv6h==NULL)\n");
		return -1;
	}

	if(ipv6h->ip6_vfc!=IPV6_VERSION)
	{
		diag_printf("ipv6h->ip6_vfc:%d.\n",ipv6h->ip6_vfc);
		return -1;
	}

	startPtr= (unsigned char *)ipv6h;
	lastPtr=startPtr+sizeof(struct ip6_hdr)+ntohs(ipv6h->ip6_plen);
	nextHeader= ipv6h ->ip6_nxt;
	ptr=startPtr+sizeof(struct ip6_hdr);
	ipv6addr[0] = ntohl(ipv6h->ip6_dst.s6_addr32[0]);
	ipv6addr[1] = ntohl(ipv6h->ip6_dst.s6_addr32[1]);
	ipv6addr[2] = ntohl(ipv6h->ip6_dst.s6_addr32[2]);
	ipv6addr[3] = ntohl(ipv6h->ip6_dst.s6_addr32[3]);

	while(ptr<lastPtr)
	{
		switch(nextHeader)
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
				/*parse hop-by-hop option*/
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+2;

				while(ptr<(startPtr+extensionHdrLen+sizeof(struct ip6_hdr)))
				{
					optionType=ptr[0];
					/*pad1 option*/
					if(optionType==0)
					{
						ptr=ptr+1;
						continue;
					}

					/*padN option*/
					if(optionType==1)
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}

					/*router altert option*/
					if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
					{
						ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
						ptr=ptr+4;
						continue;
					}

					/*other TLV option*/
					if((optionType!=0) && (optionType!=1))
					{
						optionDataLen=ptr[1];
						ptr=ptr+optionDataLen+2;
						continue;
					}


				}

				break;

			case ROUTING_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
                             ptr=ptr+extensionHdrLen;
				break;

			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;

			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;

			case ICMP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if((ptr[0]==MLD_QUERY) ||(ptr[0]==MLDV1_REPORT) ||(ptr[0]==MLDV1_DONE) ||(ptr[0]==MLDV2_REPORT))
				{
					return ICMP_PROTOCOL;

				}
				break;

			case PIM_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				if(IS_IPV6_PIM_ADDR(ipv6addr))
				{
					return PIM_PROTOCOL;
				}

				break;

			case MOSPF_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;

				if(IS_IPV6_MOSPF_ADDR1(ipv6addr) || IS_IPV6_MOSPF_ADDR2(ipv6addr))
				{
					return MOSPF_PROTOCOL;
				}
				break;

			case TCP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return TCP_PROTOCOL;

				break;

			case UDP_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				return UDP_PROTOCOL;

				break;

			default:
				/*not ipv6 multicast protocol*/
				return -1;
				break;
		}

	}
	return -1;
}
int rtl_isHopbyHop(struct ip6_hdr* ipv6h)
{
	if(ipv6h==NULL)
	{
		diag_printf("(ipv6h==NULL)\n");
		return 0;
	}

	if(ipv6h->ip6_vfc!=IPV6_VERSION)
	{
		diag_printf("ipv6h->ip6_vfc:%d.\n",ipv6h->ip6_vfc);
		return 0;
	}


	if(ipv6h ->ip6_nxt==HOP_BY_HOP_OPTIONS_HEADER)
		return 1;
	else
		return 0;
}


#endif
static inline void re865x_relayTrappedMCast(struct sk_buff *skb, unsigned int vid, unsigned int pid,unsigned int mcastFwdPortMask, unsigned int keepOrigSkb)
{
	//rtl_nicTx_info	nicTx;
 	struct sk_buff *skb2=NULL;

    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct sk_buff *skb_wan=NULL;
	rtl_nicTx_info	nicTx_wan;
	#endif
	unsigned int txPortlist=0;
	
	txPortlist = ((struct re_private *)skb->dev->priv)->port;
	if(mcastFwdPortMask==0)
	{
		return;
	}

	if(keepOrigSkb==TRUE)
	{
		skb2= skb_clone(skb, GFP_ATOMIC);
	}
	else
	{
		skb2=skb;
	}

       if(skb2!=NULL)
       {
       #if 0
       	nicTx.txIdx=0;
#if defined(CONFIG_RTL_QOS_PATCH)|| defined(CONFIG_RTK_VOIP_QOS)
	if(((struct sk_buff *)skb)->srcPhyPort == QOS_PATCH_RX_FROM_LOCAL){
		nicTx.priority = QOS_PATCH_HIGH_QUEUE_PRIO;
		nicTx.txIdx=RTL865X_SWNIC_TXRING_MAX_PKTDESC-1;	//use the highest tx ring index, note: not RTL865X_SWNIC_TXRING_HW_PKTDESC-1
	}
#endif
		#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	//fix tim; upnp
    if( vlan_enable && vlan_bridge_enable && vlan_bridge_multicast_enable)
	{
		if((vid == vlan_bridge_tag)&& (mcastFwdPortMask & RTL_WANPORT_MASK))
		{
			mcastFwdPortMask &= (~RTL_WANPORT_MASK);
			skb_wan = skb_copy(skb, GFP_ATOMIC);
			 if(skb_wan!=NULL)
		     {
		       	nicTx_wan.txIdx=0;
                nicTx_wan.vid = vlan_bridge_multicast_tag;
				nicTx_wan.portlist = RTL_WANPORT_MASK;
				nicTx_wan.srcExtPort = 0;
				nicTx_wan.flags = (PKTHDR_USED|PKT_OUTGOING);
				// flush cache 0515 by tim
				_dma_cache_wback_inv((unsigned long) skb_wan->data, skb_wan->len);
				if (RTL_swNic_send((void *)skb_wan, skb_wan->data, skb_wan->len, &nicTx_wan) < 0)
				{
					dev_kfree_skb_any(skb_wan);
				}

			}
		}
	}
#endif
		
#ifdef TX_SCATTER
			if (skb2->list_num > 0)
				skb2->len = skb2->total_len;
			else {
				skb2->list_buf[0].buf = skb->data;		
				skb2->list_buf[0].len = skb->len;
				skb2->list_num = 1;
				skb2->total_len = skb->len;
			}
#endif
		#if 0
		nicTx.vid = vid;
		nicTx.portlist = mcastFwdPortMask;	//portlist->skb->cb[0]
		nicTx.srcExtPort = 0;
		nicTx.flags = (PKTHDR_USED|PKT_OUTGOING);
		#endif
		
		txPortlist &= mcastFwdPortMask;
		//diag_printf("txPortlist:%x,mcastFwdPortMask=%x.[%s]:[%d]\n",txPortlist,mcastFwdPortMask,__FUNCTION__,__LINE__);

		_dma_cache_wback_inv((unsigned long)skb2->data, skb2->len);
		
		if (RTL_swNic_send((void *)skb2, skb2->data, skb2->len, vid,txPortlist) < 0)
		{
			dev_kfree_skb_any(skb2);
		}

	}
	return;
}

#if defined (CONFIG_NETFILTER)
unsigned int (*IgmpRxFilter_Hook)(struct sk_buff *skb,
	     unsigned int hook,
	     const struct net_device *in,
	     const struct net_device *out,
	     struct xt_table *table);
EXPORT_SYMBOL(IgmpRxFilter_Hook);

static bool rtl_MulticastRxFilterOff(struct sk_buff *skb, int ipversion)
{
	bool ret =  true;
	if(IgmpRxFilter_Hook == NULL)
	{
		DEBUG_ERR("IgmpRxFilter_hook is NULL\n");
		return false;
	}
	if(ipversion ==4)
		skb->network_header = (sk_buff_data_t)re865x_getIpv4Header(skb->data);
	else if(ipversion ==6)
		skb->network_header = (sk_buff_data_t)re865x_getIpv6Header(skb->data);
	else
		return ret;//error shouldn't happen
#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = (sk_buff_data_t)(skb->data - skb->head);
#else
	skb->mac_header = (sk_buff_data_t)skb->data;
#endif

	//data should point to l3 header while doing iptables check
		skb->data = skb->data+ETH_HLEN;

	if(ipversion ==4)
	{
		struct net_device	*origDev=skb->dev;
		if((skb->dev->br_port!=NULL))
		{
			skb->dev=__dev_get_by_name(dev_net(skb->dev),RTL_PS_BR0_DEV_NAME);
			
		}
		ret = ((IgmpRxFilter_Hook(skb, NF_INET_PRE_ROUTING,  skb->dev, NULL,dev_net(skb->dev)->ipv4.iptable_filter)) !=NF_ACCEPT);
		skb->dev=origDev;
	}
	else if(ipversion ==6)
		ret = false;//ipv6 hava no iptables rule now

	if(ret)
	{
		DEBUG_ERR(" filter a v%d pkt\n", ipversion);
	}
	// return point to l2 header
	skb->data = skb->data-ETH_HLEN;
	return ret;
}
#endif


__MIPS16
int  rtl_MulticastRxCheck(struct sk_buff *skb,struct re_private *cp_this,unsigned int vid,unsigned int pid)
{

	unsigned int  vlanRelayPortMask=0;
	unsigned int  igmpRelayPortMask;
	unsigned int  vlanPortMask=0;
	struct iphdr *iph=NULL;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo nicMCastFwdInfo;
	struct rtl_multicastFwdInfo br0MCastFwdInfo;

#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct ip6_hdr *ipv6h=NULL;
#endif
	unsigned int l4Protocol=0;
	int ret=FAILED;
	unsigned char reserved=0;
#if 0//defined (CONFIG_RTL_MLD_SNOOPING)
	struct dev_priv *cp_this=info->priv;
#endif
	//int vid=info->vid;
	//int pid=info->pid;
#if defined(CONFIG_RTL_8197F)
	unsigned int pvid;
	if ((vid==0) && (rtl819x_getSwEthPvid(pid, &pvid)==SUCCESS))
		vid = pvid;
#endif
	
	
	if((skb->data[0] &0x01) ==0)
	{
		return -1;
	}
	
	
	
	/*set flooding port mask first*/
	
	vlanPortMask=vlanPortmask_get(vid);
	
	vlanRelayPortMask = vlanPortMask & (~(1<<pid))& cp_this->port & ((1<<RTL8651_MAC_NUMBER)-1);
	//diag_printf("vlanRelayPortMask:%x,vid:%d,VlanPortMask:%x,pid:%x,cp_this->port:%x,[%s]:[%d].\n",vlanRelayPortMask,vid,vlanPortMask,pid,cp_this->port,__FUNCTION__,__LINE__);

	if((skb->data[0]==0x01) && (skb->data[1]==0x00)&& (skb->data[2]==0x5e))
	{
		
		
		#if defined(CONFIG_RTL_VLAN_SUPPORT)
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				skb->srcVlanId=vid;
				skb->srcPort=pid;
			#endif
			if(rtl_vlan_support_enable)
			{
				/*let bridge handle it*/
				
				return 0;
			}
		#else
			if(rtl_vlan_support_enable)
			{
				/*let bridge handle it*/
				
				return 0;
			}
			else
			
		#endif
		#endif
		{
				
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				skb->srcVlanId=vid;
				skb->srcPort=pid;
			#endif
		}
		
		if(rtl_isWanDev(cp_this))
			return -1;
		
		/*hardware ip multicast table will trap 0x01-00-5e-XX-XX-XX type packets*/
		/*how about other packets not trapped by hardware multicast table?---->we assume it has been flooded by l2 table*/
		iph=re865x_getIpv4Header(skb->data);
		if(iph!=NULL)
		{
			/*for upnp*/
			if((uint32)(iph->daddr) == htonl(0xEFFFFFFA)){
				reserved = 1;
			}	
			/*udp or tcp packet*/
			l4Protocol=iph->protocol;
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{

				/*relay packets which are trapped by hardware multicast table*/
				#if 1//defined (CONFIG_RTL_HARDWARE_MULTICAST)
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT//fim tim
				if(vlan_enable && vlan_bridge_enable && !strcmp(cp_this->dev->name,RTL_PS_ETH_NAME_ETH2))
				{
					if(igmpsnoopenabled && (nicIgmpModuleIndex_2!=0xFFFFFFFF))
					{
						multicastDataInfo.ipVersion=4;
						multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
						multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);

						multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
						multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
						
						ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex_2, &multicastDataInfo, &nicMCastFwdInfo);
						vlanRelayPortMask &= nicMCastFwdInfo.fwdPortMask ;
						if(ret==SUCCESS)
						{

						}
						else
						{
							ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex_2, &multicastDataInfo, &br0MCastFwdInfo);
							if(ret==SUCCESS)
							{
								/*there is wireless client,can not flooding in vlan */
								vlanRelayPortMask=0;
							}
						}

					}
				}
				else
				#endif
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					multicastDataInfo.ipVersion=4;
					multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
					multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
					
					multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
					multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
					
					ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &nicMCastFwdInfo);
					
					vlanRelayPortMask &= nicMCastFwdInfo.fwdPortMask ;
					//diag_printf("vlanRelayPortMask:%x,fwd:%x,[%s]:[%d].\n", vlanRelayPortMask,nicMCastFwdInfo.fwdPortMask,__FUNCTION__,__LINE__);
					if(ret==SUCCESS)
					{

					}
					else
					{
						ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &br0MCastFwdInfo);
						if(ret==SUCCESS)
						{
							/*there is wireless client,can not flooding in vlan */
							vlanRelayPortMask=0;
						}

					}
				}
				re865x_relayTrappedMCast( skb, vid, cp_this->port, vlanRelayPortMask, TRUE);
				
				#endif/*end of CONFIG_RTL_HARDWARE_MULTICAST*/
			}
			else if((l4Protocol==IPPROTO_IGMP)&&(reserved == 0))
			{
				
                #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
				if(vlan_enable && vlan_bridge_enable && !strcmp(cp_this->dev->name,RTL_PS_ETH_NAME_ETH2))
				{
					if(igmpsnoopenabled && (nicIgmpModuleIndex_2!=0xFFFFFFFF))
					{
						rtl_igmpMldProcess(nicIgmpModuleIndex_2, skb->data, pid, &igmpRelayPortMask);
						//just flooding
						igmpRelayPortMask &= vlanRelayPortMask;
					}
				}
				else
				#endif
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
				
					/*igmp packet*/
				#if defined (CONFIG_NETFILTER)
					if(rtl_MulticastRxFilterOff(skb, 4) == true)
						return 0;//filter by iptables
				#endif
					ret=rtl_igmpMldProcess(nicIgmpModuleIndex, skb->data, pid, &igmpRelayPortMask);
					igmpRelayPortMask &= vlanRelayPortMask;
					
				}

				re865x_relayTrappedMCast( skb, vid, cp_this->port, igmpRelayPortMask, TRUE);

			}
			else
			{

				re865x_relayTrappedMCast( skb, vid, cp_this->port, vlanRelayPortMask, TRUE);
			}
		}


	}
#if defined (CONFIG_RTL_MLD_SNOOPING)

	else if ((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xff))
	{
		#if defined(CONFIG_RTL_HARDWARE_MULTICAST)&& defined(CONFIG_RTL_8197F)
		skb->srcVlanId=vid;
		skb->srcPort=pid;
		#endif
		
		if(mldSnoopEnabled!=TRUE)
		{
			return 0;
		}
		if(rtl_isWanDev(cp_this))
			return -1;
		
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		if ((rtl_isEthPassthruFrame(skb->data)!=FAILED)&&(rtl_isWanDev(cp_this)==TRUE))
		{
			/*don't relay it,let linux protocol stack bridge handle it*/
			return 0;
		}
		#endif

		#if defined(CONFIG_RTL_VLAN_SUPPORT)
		if(rtl_vlan_support_enable)
		{
			/*let bridge handle it*/
			return 0;
		}
		#endif

		/*when enable mld snooping, gateway will add acl to trap packet with dmac equal to 0x33-33-xx-xx-xx-xx */
		ipv6h=re865x_getIpv6Header(skb->data);
		if(ipv6h!=NULL)
		{
			l4Protocol=re865x_getIpv6TransportProtocol(ipv6h);
			/*udp or tcp packet*/
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{

				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					multicastDataInfo.ipVersion=6;
					memcpy(&multicastDataInfo.sourceIp, &ipv6h->ip6_src, 16);
					memcpy(&multicastDataInfo.groupAddr, &ipv6h->ip6_dst, 16);

					multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
					multicastDataInfo.sourceIp[1] = ntohl(multicastDataInfo.sourceIp[1]);
					multicastDataInfo.sourceIp[2] = ntohl(multicastDataInfo.sourceIp[2]);
					multicastDataInfo.sourceIp[3] = ntohl(multicastDataInfo.sourceIp[3]);
					multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
					multicastDataInfo.groupAddr[1] = ntohl(multicastDataInfo.groupAddr[1]);
					multicastDataInfo.groupAddr[2] = ntohl(multicastDataInfo.groupAddr[2]);
					multicastDataInfo.groupAddr[3] = ntohl(multicastDataInfo.groupAddr[3]);

					ret=rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &nicMCastFwdInfo);
					//diag_printf("fwd:%x,[%s]:[%d].\n", nicMCastFwdInfo.fwdPortMask,__FUNCTION__,__LINE__);
					vlanRelayPortMask &= nicMCastFwdInfo.fwdPortMask ;
					if(ret==SUCCESS)
					{

					}
					else
					{
						ret=rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &br0MCastFwdInfo);
						if(ret==SUCCESS)
						{
							/*there is wireless client,can not flooding in vlan */
							vlanRelayPortMask=0;
						}
					}
				}

				re865x_relayTrappedMCast( skb, vid, cp_this->port, vlanRelayPortMask, TRUE);

			}
			else if(l4Protocol==IPPROTO_ICMPV6)
			{
				//diag_printf("IPPROTO_ICMPV6,[%s]:[%d].\n", __FUNCTION__,__LINE__);
				/*icmp packet*/
				if(igmpsnoopenabled && (nicIgmpModuleIndex!=0xFFFFFFFF))
				{
					rtl_igmpMldProcess(nicIgmpModuleIndex, skb->data, pid, &igmpRelayPortMask);
					igmpRelayPortMask &= vlanRelayPortMask;
				}

				re865x_relayTrappedMCast( skb, vid, cp_this->port,igmpRelayPortMask, TRUE);

			}
			else
			{
				re865x_relayTrappedMCast( skb, vid, cp_this->port, vlanRelayPortMask, TRUE);
			}
		}
		else
		{
			re865x_relayTrappedMCast( skb, vid, cp_this->port, vlanRelayPortMask, TRUE);
		}

	}
#endif
	else
	{
		#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST)
		skb->srcVlanId=0;
		skb->srcPort=0xFFFF;
		#endif
	}

	#if 0
	if(rx_skb_queue.qlen < (rtl865x_maxPreAllocRxSkb/3))
	{
		refill_rx_skb();
	}
	#endif

	return 0;
}

#define BR_MODULE	1
#define ETH_MODULE  0

#define IGMP_SNOOPING_ENABLE_FLAG 1
#define MLD_SNOOPING_ENABEL_FLAG 0

uint32 rtl_getIgmpModuleIndex(int moduleFlag)
{
	uint32 IgmpModuleIndex=0xFFFFFFFF;
	
	if (igmpsnoopenabled)
	{
		if (moduleFlag == BR_MODULE)
			IgmpModuleIndex = brIgmpModuleIndex;
		else if (moduleFlag == ETH_MODULE)
			IgmpModuleIndex = nicIgmpModuleIndex;

	}
	
	return IgmpModuleIndex;
}
uint32 rtl_getIgmpSnoopingEnable(int snoopFlag)
{
	uint32 enable=0;
	
	if (snoopFlag == IGMP_SNOOPING_ENABLE_FLAG)
		enable = igmpsnoopenabled;
#if defined (CONFIG_RTL_MLD_SNOOPING)
	else if (snoopFlag == MLD_SNOOPING_ENABEL_FLAG)
		enable = mldSnoopEnabled;
#endif
	return enable;
		
}

#endif	/*end of CONFIG_RTL865X_IGMP_SNOOPING*/
static inline unsigned compare_ether_addr(const u8 *addr1, const u8 *addr2)
{
	const u16 *a = (const u16 *) addr1;
	const u16 *b = (const u16 *) addr2;

	//BUILD_BUG_ON(ETH_ALEN != 6);
	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) != 0;
}


#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
int old_passThru_flag = 0;
char passThru_flag[2] = { 0x30, 0};
#define IPV6_FRAME 0
#define PPPOE_FRAME 1
static const unsigned char passthru_vlan_mac[6] = { 0x00, 0x12, 0x34, 0x56, 0x78, 0x90 };

int32 rtl_isEthPassthruFrame(uint8 *data)
{
	int	ret;

	ret = FAILED;
	
	/*
	diag_printf("data:%x-%x-%x-%x-%x-%x,%x-%x-%x-%x-%x-%x,[%s]:[%d].\n",
	data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],__FUNCTION__,__LINE__);
	*/
	if (old_passThru_flag)
	{
		if (old_passThru_flag&IP6_PASSTHRU_MASK)
		{
			if ((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_IPV6)) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_IPV6))))
			{
				//diag_printf("is ipv6PassthruFrame.[%s]:[%d].\n",__FUNCTION__,__LINE__);
				ret = IPV6_FRAME;
			}
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if (old_passThru_flag&PPPOE_PASSTHRU_MASK)
		{
			if (((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_DISC))) ||
				((*((uint16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&((*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_SES))||(*((uint16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_DISC)))))
			{
				//diag_printf("is PPPOEPassthruFrame.[%s]:[%d].\n",__FUNCTION__,__LINE__);
				ret = PPPOE_FRAME;
			}
		}
		#endif
	}

	return ret;
}
int rtl_getPassthruMask(void)
{

	return old_passThru_flag;
}
#endif
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
static struct net_device *pVirtualDev = NULL;//for bridge vlan
static int rtl_bridge_wan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device *wan_dev = NULL;
    int i = 0;
    unsigned char lan_macaddr[6] = {0};
    rtl_getLanMacAddr(lan_macaddr);
	if(!memcmp(lan_macaddr,skb->data+6, 6)) {
		dev_kfree_skb_any(skb);
		return 0;
	}
    if (skb->data+24 == 0x02)
    {  //discard igmp packet xmit from eth7
    	dev_kfree_skb_any(skb);
        return 0;
    }
    #if 0
	diag_printf("------------[%s][%d]-skb->dev[%s]\n", __FUNCTION__, __LINE__, skb->dev->name);
    diag_printf("tx dac: 0x%x:%x:%x:%x:%x:%x smac 0x%x:%x:%x:%x:%x:%x \n", 
                    skb->data[0],
                    skb->data[1], 
                    skb->data[2], 
                    skb->data[3], 
                    skb->data[4], 
                    skb->data[5], 
                    skb->data[6], 
                    skb->data[7], 
                    skb->data[8], 
                    skb->data[9], 
                    skb->data[10], 
                    skb->data[11]);
    
    #endif
	for (i = 0; i < ETH_INTF_NUM; i++)
    {
        if (reNet[i] && (((struct re_private *)reNet[i]->priv)->is_wan))
            wan_dev = reNet[i];
    }

	if(wan_dev){
		skb->dev = wan_dev;
		//wan_dev->netdev_ops->ndo_start_xmit(skb, wan_dev);
        (wan_dev->hard_start_xmit)(skb, wan_dev);
    }
    
	return 0;
}
static inline void rtl_processRxToProtcolStack(struct sk_buff *skb, struct dev_priv *cp_this)
{
#ifdef __KERNEL__
	skb->protocol = eth_type_trans(skb, skb->dev);
	skb->ip_summed = CHECKSUM_NONE;
	//printk("[%s][%d]-skb->dev[%s],proto(0x%x)\n", __FUNCTION__, __LINE__, skb->dev->name,skb->protocol);
#if defined(RX_TASKLET)
	#if defined(CONFIG_RTL_LOCAL_PUBLIC)
	skb->localPublicFlags = 0;
	#endif
	#if defined(CONFIG_RTL_FAST_BRIDGE)
	skb->fast_br_forwarding_flags = 0;
	#endif
	
	netif_receive_skb(skb);    
#endif
#else	/*	defined(RX_TASKLET)	*/
	netif_rx(skb);
#endif	/*	defined(RX_TASKLET)	*/
}
struct net_device* rtl_get_virtual_dev_for_bridge_vlan(void)
{
    if (pVirtualDev)
        return pVirtualDev;
    else
        return NULL;
}
void rtl_getLanMacAddr(unsigned char *lan_macaddr)
{
    if (lan_macaddr != NULL)
        memcpy(lan_macaddr,reNet[0]->dev_addr, 6);
    return;
}

void rtl_getWanMacAddr(unsigned char *wan_macaddr)
{
    int i;
    if (wan_macaddr != NULL)
    {
        for (i = 0; i < ETH_INTF_NUM; i++)
        {
            if (reNet[i] && (((struct re_private *)reNet[i]->priv)->is_wan))
                break;
                
        }
        if (i != ETH_INTF_NUM)
            memcpy(wan_macaddr,reNet[i]->dev_addr, 6);
    }
    return;
}

#endif

#if defined(CONFIG_RTL_DNS_TRAP)
extern int32 add_acl_for_dns_trap(uint32 index);
#if defined(CONFIG_RTL_8367R_SUPPORT)
extern int rtl_8367_add_acl_for_dns(unsigned int acl_idx);
#endif


#endif
#if defined(CONFIG_RTL_DNS_TRAP) || defined(__CONFIG_APCLIENT_DHCPC__)
int is_skb_dns_packet(struct sk_buff* skb)
{
    if(!skb)
		return 0;
	if(*(unsigned short *)(skb->data+ETH_ALEN*2)==htons(0x0800) &&
		*(unsigned char *)(skb->data+0x17) == 17 &&
		*(unsigned short *)(skb->data+0x24) == htons(53))
			return 1;
	return 0;
}
#endif
//---------------------------------------------------------------------------
__IRAM_SECTION_
__MIPS16
static void interrupt_dsr_rx(unsigned long task_priv)
{
	struct re_private *cp = (struct re_private *)task_priv;
	struct re_private *cp_next;
	struct re_private *cp_this;
	struct sk_buff *skb = NULL;
 	int len;
 	unsigned int vid,pid;
	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	struct re_private *cp2;
	int i2;
	#endif
	#if defined(CONFIG_RTL_819X)	//jwj:20120821
	int i;
	#endif
#ifdef PREVENT_BCAST_REBOUND	
	unsigned char* DA;
	unsigned long  current_time = jiffies;
	struct bcast_tr_s *bacst_Ptr = &bcast ;	
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	#define MAX_ADDR_LEN	32		/* Largest hardware address length */
	unsigned char *dest_mac;
	int isPassthru= FAILED;
#endif
#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	int ret_check = FAILED;
#endif 	
#ifdef CONFIG_RTK_VOIP
 	unsigned long start_time = jiffies;
#endif
	//unsigned rx_work = 10000;
	unsigned rx_work = 256;
	
#ifdef BR_SHORTCUT
#ifdef CONFIG_WIRELESS_LAN_MODULE
#else
	extern struct net_device *get_shortcut_dev(unsigned char *da);
#endif
	struct net_device *dev;
#endif	

#ifdef ETH_NEW_FC
	if (!cp->opened)
		return;	
#endif
// for debug ----------------------
//	cp_next = NEXT_CP(cp);
	if (NEXT_DEV(cp) && NEXT_CP(cp)->opened)
		cp_next = NEXT_CP(cp);
	else
		cp_next = NULL;
//---------------------------------		

	while (rx_work--) {   		
#ifdef CONFIG_RTK_VOIP
		extern char chanEnabled[2];

		if ( (jiffies - start_time) > 1 && (chanEnabled[0] || chanEnabled[1]) ){
#ifdef RX_TASKLET		
			tasklet_hi_schedule(&cp->rx_dsr_tasklet);			
#endif	
			break;
		}
#endif
		if (RTL_swNic_receive((void **)&skb, (unsigned int *)&len, &vid, &pid) !=  0)			
			break;

		/*
		
		diag_printf("\nskb=%p\n", skb);
		diag_printf("len=%d\n", len);
		{
				int i;
				unsigned char *data;
				data = (unsigned char *)skb->data;
				for (i=0;(i<len);i++){
					if (i%16 ==0)
 					diag_printf("\n%02X ", data [i]);
					else
						diag_printf("%02X ", data [i]);
					
					}
		}
		diag_printf("\n");
		
		*/	
	
#ifdef __KERNEL__
		// Verify IP/TCP/UDP checksum
		if (skb->ip_summed != 0) {
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)   
	if ((vid != WAN_VID) || 
    	(((ntohl(*(unsigned short *)(skb->data + 12)) != 0x0800) || (*(skb->data + 23) != 1 /* IPPROTO_ICMP */)) &&
     	((ntohl(*(unsigned short *)(skb->data + 12)) != 0x8864) || (*(skb->data + 31) != 1 /* IPPROTO_ICMP */)))
      )
 #endif
 		{
			dev_kfree_skb_any(skb);
			continue;
		}
}
#endif		

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (L2_table_disabled) {		
		for (i2=0; i2<5; i2++) {			
			cp2 = reNet[i2]->priv;	
			if (cp2->opened && cp2->port == pid) {
				cp_this = cp2;
				break;
			}			
		}
		if (i2 != 5) 
			goto rx_packet;					
		else {
			dev_kfree_skb_any(skb);
			continue;			
		}		
	}
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	//memcpy(dest_mac, skb->data, 6);
	dest_mac=skb->data;
#endif

#ifdef CONFIG_RTL865X_HW_TABLES
		/* for the issue: WAN port PC can not ping br0 (192.168.1.254) in Bridge mode */
    		if (cp->opened && (cp->port & pid))				
       	 		cp_this = cp;
		else if (cp_next->opened && (cp_next->port & pid))
			cp_this = cp_next;
		#ifdef CONFIG_RTL865X_HW_PPTPL2TP
		//else if (vid == 10)
		else if (pid & BIT(7))
			cp_this = cp;
		#endif
	    	else {    
			dev_kfree_skb_any(skb);
			continue;
    		}
#else
#if defined(CONFIG_RTL_819X)	//jwj:20120821
		for(i = 0; i < ETH_INTF_NUM; i++)
		{
			if (reNet[i]) {
				cp = ((struct re_private *)reNet[i]->priv);
				if(cp && cp->opened && (cp->port & (1<<pid)))
				{
					//diag_printf("=========%s(%d),cp(%s),i(%d), portmask is 0x%x\n",__FUNCTION__,__LINE__,cp->dev->name,i, cp->port);
					cp_this = cp;
					break;
				}
			}
		}

		if(ETH_INTF_NUM==i)
		{
			dev_kfree_skb_any(skb);
			continue;
		}
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	
		else if(((isPassthru=rtl_isEthPassthruFrame(skb->data))!=FAILED)&&(rtl_isWanDev(cp)==TRUE))
		{
			
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
			if((IPV6_FRAME==isPassthru)||
			((PPPOE_FRAME==isPassthru)&&(compare_ether_addr((char* )cp->dev->dev_addr, (char*)dest_mac))))
#endif
			{
			cp_this = (struct re_private *)pDev->priv;
			cp_this->isPdev=TRUE;
			
			}	
		}
	
#endif
		
#else//defined(CONFIG_RTL_819X)
    		if (cp->opened && cp->vid==vid)				
       	 		cp_this = cp;
		else if (cp_next && cp_next->opened && cp_next->vid==vid)
			cp_this = cp_next;
		#ifdef CONFIG_RTK_VOIP_WAN_VLAN         
	    	else if (cp_next->opened && vid==wan_vlan_id_proto)
			cp_this = cp_next;
		else if (cp_next->opened && vid==wan_vlan_id_video)
			cp_this = cp_next;
		else if (cp_next->opened && vid==wan_vlan_id_data)
			cp_this = cp_next;
		#endif
		
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
			
		else if (((isPassthru=rtl_isEthPassthruFrame(skb->data))!=FAILED)&&(rtl_isWanDev(cp)==TRUE))
		{
			
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
			if((IPV6_FRAME==isPassthru)||
			((PPPOE_FRAME==isPassthru)&&(compare_ether_addr((char* )cp->dev->dev_addr, (char*)dest_mac))))
#endif
			{
			cp_this = (struct re_private *)pDev->priv;
			cp_this->isPdev=TRUE;
			
			}	
		}
#endif
	    	else {    
			#if defined(CONFIG_RTK_GUEST_ZONE)
			if (L2_table_disabled) {
				for (i2 = 2; i2 < 5; i2++) {
					cp2 = reNet[i2]->priv;
					if (cp2->opened && cp2->vid == vid) {
						cp_this = cp2;
						break;
					}
				}
				if (i2 >= 5) {
					dev_kfree_skb_any(skb);
					continue;
				}				
			}
			else
			#endif
			{
			dev_kfree_skb_any(skb);
			continue;
			}
    		}
#if 0	// it is no need since we disable EN_UNUNICAST_TOCPU
#if !defined(CONFIG_RTL8197B_PANA) && !defined(CONFIG_RTL865X_PANAHOST)
		if  (memcmp(&skb->data[6], cp_this->dev->dev_addr, 6)== 0)// check source mac
		{
                        //printk("source mac is device self\n");
                        dev_kfree_skb_any(skb);
                       continue;
		}
#endif
#endif
#endif
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
rx_packet:
#endif
#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
		skb->len = 0;
#endif
		skb_put(skb, len);
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)

		skb->dev= cp_this->isPdev? pDev:cp_this->dev;

		//diag_printf("----dev:%s,[%s]:[%d].\n",skb->dev->name,__FUNCTION__,__LINE__);
#else
		skb->dev = cp_this->dev;
#endif		
#ifdef CONFIG_RTL8196_RTL8366
		if (!memcmp(cp_this->dev->dev_addr, &skb->data[6], 6)) {
			dev_kfree_skb_any(skb);
			continue;
		}
		if (*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_8021Q)) 
		{
			uint8 *data = skb->data;
			memmove(data + VLAN_HLEN, data, 2 * VLAN_ETH_ALEN);
			skb->len -= 4;
			skb->data += 4;
		}		
#endif
   		

#ifdef CONFIG_RTK_VOIP_WAN_VLAN         
		if (*(uint16*)(&(skb->data[12])) == htons(0x8100))
		{
			//printk("Get VLAN tagged packet\n");
			memmove(
				skb->data+4,
				skb->data,
				2*sizeof(ether_addr_t)
			);
			skb_pull(skb, 4);
		}
#endif	

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
		if (pid & BIT(7)) {
			rtl865x_pptpl2tp_extport_recv(skb);
			continue;
		}
		skb->cb[0] = '\0';
#endif

		cp_this->net_stats.rx_packets++;	
		cp_this->net_stats.rx_bytes += skb->len;	
#ifdef __KERNEL__
		cp_this->dev->last_rx = jiffies;
#endif		
		
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
		cp_this->rx_byte_cnt += skb->len;
#endif
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		cp_this->rx_byte_cnt += skb->len;
#endif

#if !defined(CONFIG_RTL8197B_PANA) && !defined(CONFIG_RTL865X_PANAHOST)
		// drop the packet that SA is my MAC addr
#if 0//defined(CONFIG_RTL_HARDWARE_NAT)
		if  ((memcmp(&skb->data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN)==0)||PKTHDR_EXTPORT_MAGIC2==vid||PKTHDR_EXTPORT_MAGIC==vid)// check source mac
#else
		if (!memcmp(cp_this->dev->dev_addr, &skb->data[6], 6)) 
#endif
		{
			#if 0//defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			if ((PKTHDR_EXTPORT_MAGIC!=vid)||(info->pid!=PKTHDR_EXTPORT_P3))
			#endif
			{
				dev_kfree_skb_any(skb);
				continue;
			}
		}
#endif

#ifdef PREVENT_BCAST_REBOUND
	DA = skb->data;
	if(DA[0] & 0x01){
			int i3;
			int found = 0;			
			//panic_printk("\n<===eth rx===\n");						
			for(i3=0 ; i3 < P_BCAST_NUM ; i3++){				
				if (memcmp(&skb->data[6], &bacst_Ptr->entry[i3].BCAST_SA , 6)== 0
					&& (UINT32_DIFF(current_time , bacst_Ptr->entry[i3].time_stamp) < 3000 ))
				{
                       //panic_printk("BROADCAST packet rebound ; this source mac must be filter\n");
                       dev_kfree_skb_any(skb);
					   found = 1;
					   break;	

				}
			}			
			if(found == 1)	
				continue;

		
	}
#endif	

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (cp_this->vlan_setting.global_vlan) {
		if (rx_vlan_process(cp_this->dev, &cp_this->vlan_setting, skb)){
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);		
			continue;
		}
	}
#endif
	#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	ret_check = rtl_qos_rate_limite_check(RTL_RX_DIR, skb->dev->name, skb);
	if (ret_check ==  SUCCESS){
		// upload traffic control of wired, added by zhuhuan on 2016.03.01
		dev_kfree_skb_any(skb);		
		continue;
	}
	else{
		
	}
	#endif

#ifdef CONFIG_RTL_VLAN_SUPPORT
	if (rtl_vlan_support_enable) {
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        struct sk_buff *new_skb = NULL;
		int vlan_check;
        
		vlan_check = rtl_vlanIngressProcess(skb, cp_this->dev->name, &new_skb);
        cp_this = (struct re_private *)skb->dev->priv;
        if((vlan_check==0) && new_skb){
			rtl_processRxToProtcolStack(new_skb, cp_this);
		}else if(vlan_check == 1){
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);
			continue;
		}else if((vlan_check == 2) && new_skb){
			dev_kfree_skb_any(new_skb);
		}
        else if (vlan_check < 0){
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);		
			continue;
		}
        #else
		if (rtl_vlanIngressProcess(skb, cp_this->dev->name) < 0){
			cp_this->net_stats.rx_dropped++;
			dev_kfree_skb_any(skb);		
			continue;
		}
        #endif
	}
#endif

#ifndef CONFIG_RTL8196_RTL8366	//jwj:20120820
	if (*((unsigned short *)(skb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_8021Q)) 
	{
		uint8 *data = skb->data;
		//memmove(data + VLAN_HLEN, data, 2 * ETH_ALEN);
		memmove(data + 4, data, 2 * ETH_ALEN);
		skb->len -= 4;
		skb->data += 4;
	}
#endif

#ifdef CONFIG_WIRELESS_LAN_MODULE
		if ((enable_brsc && !(skb->data[0] & 0x01) && (eth_flag & BIT(2))  && (cp_this->dev->br_port))
			&& (wirelessnet_hook_shortcut!=NULL)
			&& (((dev = wirelessnet_hook_shortcut(skb->data)) != NULL))
			) {
#if 0
			memcpy(cached_eth_addr, &skb->data[6], 6);
			cached_dev = cp_this->dev;
#else
			if (memcmp(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				//memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev = cp_this->dev;
				last_used = 0;
			}
			else if (memcmp(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				//memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev2 = cp_this->dev;
				last_used = 1;
			}
			else if (memcmp(cached_eth_addr3, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				cached_dev = cp_this->dev;
				last_used = 2;
			}					
			else if (memcmp(cached_eth_addr4, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
				cached_dev = cp_this->dev;
				last_used = 3;
			}						
			else if (last_used == 3) {
				memcpy(cached_eth_addr3, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev3 = cp_this->dev;
				last_used = 2;
			}			
			else if (last_used == 2) {
				memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev2 = cp_this->dev;
				last_used = 1;
			}		
			else if (last_used == 1) {
				memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev = cp_this->dev;
				last_used = 0;
			}
			else {
				memcpy(cached_eth_addr4, &skb->data[ETH_ALEN], ETH_ALEN);
				cached_dev4 = cp_this->dev;
				last_used = 3;				
			}
#endif						
			
			cached_aging = 30;
			skb->dev = dev;
			dev->hard_start_xmit(skb, dev);
		}
		else
#else
#ifdef BR_SHORTCUT
		if (enable_brsc && !(skb->data[0] & 0x01) &&
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)  || defined(CONFIG_RTL_ULINKER)			
				(skb->data[37] != 68) && //port 68 is dhcp dest port. In order to hack dns ip, so dhcp package can't enter bridge short cut.
#endif
#if defined(CONFIG_RTL_DNS_TRAP)
				!(is_skb_dns_packet(skb)) &&
#endif
				(eth_flag & BIT(2))  &&
#ifdef __KERNEL__				
				(cp_this->dev->br_port) &&
#else
				(IS_BRIDGE_PORT(cp_this->dev)) &&	//eth0(LAN), a workaround
#endif
#ifdef __CONFIG_APCLIENT_DHCPC__
				(skb->data[37] != 53) &&
#endif
				((dev = get_shortcut_dev(skb->data)) != NULL)) {
		#if defined(CONFIG_RTL_HARDWARE_NAT)
			if (memcmp(&skb->data[ETH_ALEN], cp_this->dev->dev_addr, ETH_ALEN))
		#endif
			{
#if 0				
				memcpy(cached_eth_addr, &skb->data[6], 6);
				cached_dev = cp_this->dev;
#else
				if (memcmp(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
					//memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev = cp_this->dev;
					last_used = 0;
				}
				else if (memcmp(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
					//memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev2 = cp_this->dev;
					last_used = 1;
				}
				else if (memcmp(cached_eth_addr3, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
					cached_dev = cp_this->dev;
					last_used = 2;
				}					
				else if (memcmp(cached_eth_addr4, &skb->data[ETH_ALEN], ETH_ALEN) == 0) {
					cached_dev = cp_this->dev;
					last_used = 3;
				}						
				else if (last_used == 3) {
					memcpy(cached_eth_addr3, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev3 = cp_this->dev;
					last_used = 2;
				}			
				else if (last_used == 2) {
					memcpy(cached_eth_addr2, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev2 = cp_this->dev;
					last_used = 1;
				}		
				else if (last_used == 1) {
					memcpy(cached_eth_addr, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev = cp_this->dev;
					last_used = 0;
				}
				else {
					memcpy(cached_eth_addr4, &skb->data[ETH_ALEN], ETH_ALEN);
					cached_dev4 = cp_this->dev;
					last_used = 3;				
				}
#endif						

			}
			skb->dev = dev;
#ifdef TX_SCATTER
			skb->list_num = 0;
#endif			
			dev->hard_start_xmit(skb, dev);	
		}
		else
#endif
#endif
		{
#ifdef CONFIG_RTL8186_KB
			skb->__unused = 0;
#endif

#ifdef DYNAMIC_ADJUST_TASKLET
			if (rx_pkt_thres > 0 && cp_this->dev->name[3] == '0') // eth0
				rx_cnt++;
#endif			

#ifdef BR_SHORTCUT
			pkt_cnt++;
#endif

#ifdef __KERNEL__
			skb->protocol = eth_type_trans(skb, cp_this->dev);
			skb->ip_summed = CHECKSUM_UNNECESSARY;
#endif			
			skb->cb[3] = (char)pid; // add for IGMP snooping, david+2008-11-05
			skb->cb[4] = (char)cp_this->vid; // add for IGMP snooping, david+2008-11-05

#ifdef __ECOS
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)

			skb->dev= cp_this->isPdev? pDev:cp_this->dev;
#else
			skb->dev = cp_this->dev;
#endif	
#endif

#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
            extern uint8 rtl_check_wanorlan(char *devname);
            skb->wanorlan = rtl_check_wanorlan(skb->dev->name); /* wan:1 lan:2 default:0 */
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
		if (1==(skb->data[0]&0x01)){
			//diag_printf("multicast pkt[%s]:[%d]!\n",__FUNCTION__,__LINE__);

			rtl_MulticastRxCheck(skb, cp_this, vid, pid);

		}
		
#endif	/*end of CONFIG_RTL865X_IGMP_SNOOPING*/

#ifdef SUPPORT_INPUT_PKT_QUEUE_VOIP
			if (skb->data[1]) //DSCP!=0
			{ 
				netif_rx_voip(skb);
			} 
			else
			{
			netif_rx(skb);			
		}
#else

#if defined(__ECOS) && defined(ISR_DIRECT)
			netif_rx(skb,  reNet[0]);
#else
			netif_rx(skb);
#endif
#endif
		}
    	}

#ifdef RX_TASKLET
    	REG32(CPUIIMR) |= RX_DONE_IE_ALL | PKTHDR_DESC_RUNOUT_IP_ALL;
#endif
}

//---------------------------------------------------------------------------
void interrupt_dsr_tx(unsigned long task_priv)
{
	int free_desc_num;
	struct re_private *cp = (struct re_private *)task_priv;
	unsigned long flags;

#ifdef ETH_NEW_FC
	if (!cp->opened)
		return;	
#endif

	save_and_cli(flags);
#ifndef __ECOS
	refill_rx_skb();
#endif

	free_desc_num = RTL_swNic_txDone(); 

	restore_flags(flags);

#ifdef __KERNEL__
    if (free_desc_num >= (txRingSize[0]/4)) {
		if (cp->opened && netif_queue_stopped(cp->dev)) 			
			netif_wake_queue(cp->dev);
		
		if (NEXT_CP(cp)->opened && netif_queue_stopped(NEXT_DEV(cp)))
			netif_wake_queue(NEXT_DEV(cp));			
   	}
#endif
		
#ifdef TX_TASKLET
    REG32(CPUIIMR) |= TX_ALL_DONE_IE_ALL;		
#endif

}

//---------------------------------------------------------------------------
#ifdef __KERNEL__
static void interrupt_isr(int irq, void *dev_instance, struct pt_regs *regs)
{
    struct net_device *dev = dev_instance;
    struct re_private *cp = dev->priv;
	unsigned int status;

	status =*(volatile unsigned int*)(CPUIISR); 
    *(volatile unsigned int *)(CPUIISR) = status; 

    if (!status || (status == 0xFFFF)) {
		spin_unlock(&cp->lock);			
		return;
	}

#ifdef ETH_NEW_FC
	if (!cp->opened)
		return;	
#endif
#if	0	//def CONFIG_RTK_PORT_HW_IGMPSNOOPING
    if (status & LINK_CHANGE_IP) {
		//printk("%s: link changed.\n",dev->name);
		interrupt_link_state_for_multicast((unsigned long)cp);
    }
#endif

/*
#ifdef CONFIG_RTK_MESH
	if (status & LINK_CHANGE_IP) {
		//printk("%s: link changed.\n",dev->name);
		interrupt_link_state_for_mesh();
	}
#endif
*/

    if (status & PKTHDR_DESC_RUNOUT_IP_ALL) {
		DEBUG_ERR("EthDrv: packet RUNOUT error!\n");
		//*(volatile unsigned int *)(CPUIIMR)|=PKTHDR_DESC_RUNOUT_IE_ALL;  
		//*(volatile unsigned int *)(CPUIISR)=PKTHDR_DESC_RUNOUT_IP_ALL;   
    }

	if (status & (RX_DONE_IP_ALL |PKTHDR_DESC_RUNOUT_IP_ALL)) {
#ifdef RX_TASKLET		
		*(volatile unsigned int *)(CPUIIMR) &= ~RX_DONE_IE_ALL;			
			tasklet_hi_schedule(&cp->rx_dsr_tasklet);			
#else	
		interrupt_dsr_rx((unsigned long)cp);	
#endif
	}		

	if (status &TX_ALL_DONE_IP_ALL) {
#ifdef TX_TASKLET		
		*(volatile unsigned int *)(CPUIIMR) &= ~TX_ALL_DONE_IP_ALL;
		tasklet_schedule(&cp->tx_dsr_tasklet);						
#else
		interrupt_dsr_tx((unsigned long)cp);
#endif			
	}			
}
#endif /* __KERNEL__ */


//---------------------------------------------------------------------------
#ifdef __ECOS
static int interrupt_isr(struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	unsigned int status;
	int call_dsr = 0; 
	int i;
	
	status =*(volatile unsigned int*)(CPUIISR); 
	*(volatile unsigned int *)(CPUIISR) = status; 

	if (!status || (status == 0xFFFF)) 
		return 0;
	
	if (status & PKTHDR_DESC_RUNOUT_IP_ALL) {
		DEBUG_ERR("%s: packet RUNOUT error!\n", __FUNCTION__);	
	}


//move to function rtl_check_8367r_link_status(), and call it in one_sec_timer
#if 0 //defined(CONFIG_RTL_8367R_SUPPORT) 
	extern int rtk_int_status_get(rtk_int_8367r_status_t* pStatusMask);
	extern int rtk_int_status_set(rtk_int_8367r_status_t pStatusMask);
	
	rtk_int_8367r_status_t status_8367r;
	memset(&status_8367r,0,sizeof(rtk_int_8367r_status_t));

	if(rtk_int_status_get(&status_8367r) == 0){
		if(status_8367r.value[0] & 1<<0 != 0){
#ifdef CONFIG_RTL_8197D_DYN_THR
			newLinkPortMask = rtl865x_getPhysicalPortLinkStatus();
			rtl819x_setQosThreshold(curLinkPortMask, newLinkPortMask);
#endif
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
		if(block_link_change ==0) {
			for(i=0; i<5; i++) {
				if((curLinkPortMask & (1<<i))==0){
					if ((newLinkPortMask & (1<<i))  != 0)	
						if(set_if_status(ETH_PORT(i),IF_UP) ==  -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
				}
				if((curLinkPortMask & (1<<i))!=0){
					if ((newLinkPortMask & (1<<i)) == 0)	
						if(set_if_status(ETH_PORT(i),IF_DOWN) == -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
				}

				if((curLinkPortMask & (1<<i)) != (newLinkPortMask & (1<<i))){
					printf("\n%s:%d calling set_if_change_status()\n", __FUNCTION__,__LINE__);
					if(set_if_change_status(ETH_PORT(i),IF_STATUS_CHANGE) == -1)
						DEBUG_ERR("%s: set ethernet port status change bit error!\n",__FUNCTION__);
				}
			}
		}	
#endif

#ifdef RTK_SYSLOG_SUPPORT
			for(i=0; i<5; i++) {
				if((curLinkPortMask & (1<<i))==0){
					if ((newLinkPortMask & (1<<i)) != 0)	
						printk("Ethernet port%d link up\n",i);
				}
				if((curLinkPortMask & (1<<i))!=0){
					if ((newLinkPortMask & (1<<i)) == 0)
						printk("Ethernet port%d link down\n",i);
				}
			}
#endif		
			curLinkPortMask=newLinkPortMask;

			status_8367r.value[0] = 0x1;
			status_8367r.value[1] = 0;
			rtk_int_status_set(status_8367r);
			
		}
	}

#endif


#ifndef CONFIG_RTL_8367R_SUPPORT	
 
	if (status & LINK_CHANGE_IP) {
		newLinkPortMask=rtl865x_getPhysicalPortLinkStatus();
		
#ifdef CONFIG_RTL_8197D_DYN_THR
		rtl819x_setQosThreshold(curLinkPortMask, newLinkPortMask);
#endif

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
		if(block_link_change == 0) {
			for(i=0; i<5; i++) {
				if((curLinkPortMask & (1<<i))==0){
					if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) != 0)	
						if(set_if_status(ETH_PORT(i),IF_UP) ==  -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
				}
				if((curLinkPortMask & (1<<i))!=0){
					if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0)	
						if(set_if_status(ETH_PORT(i),IF_DOWN) == -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
				}

				if(((curLinkPortMask>>i) & (1)) != ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp)>>4)){
					if(set_if_change_status(ETH_PORT(i),IF_STATUS_CHANGE) == -1)
						DEBUG_ERR("%s: set ethernet port status change bit error!\n",__FUNCTION__);
				}
			}
		}
#endif
#ifdef RTK_SYSLOG_SUPPORT
		for(i=0; i<5; i++) {
			if((curLinkPortMask & (1<<i))==0){
				if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) != 0)	
					printk("Ethernet port%d link up\n",i);
			}
			if((curLinkPortMask & (1<<i))!=0){
				if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0)
					printk("Ethernet port%d link down\n",i);
			}
		}
#endif
		curLinkPortMask=newLinkPortMask;
	}


#ifdef CONFIG_RTL8196C_ETH_IOT 
    if (status & LINK_CHANGE_IP) {
		uint32 agc, cb0, snr, new_port_link_sts=0, val;
		uint32 DUT_eee, Linkpartner_eee;
		int i;

		/************ Link down check ************/
		for(i=0; i<5; i++) {
			if ((REG32(PSRP0 + (i * 4)) & LinkDownEventFlag) != 0)	// !!! LinkDownEventFlag is a read clear bit, so these code must ahead of "Link up check"
				if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) == 0) {
					/*=========== ###01 ===========*/
					extern void set_gray_code_by_port(int);
					set_gray_code_by_port(i);

					/*=========== ###03 ===========*/
					// read DUT eee 100 ability
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
					rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
					rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

					// due to the RJ45 cable is plug out now, we can't read the eee 100 ability of link partner.
					// we use the variable port_linkpartner_eee to keep link partner's eee 100 ability after RJ45 cable is plug in.
					if (  ( ((DUT_eee & 0x2) ) == 0)  || ((port_linkpartner_eee & (1<<i)) == 0) )  {
						rtl8651_getAsicEthernetPHYReg( i, 21, &val );
						val = val & ~(0x4000);
						rtl8651_setAsicEthernetPHYReg( i, 21, val );
					}
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
					if(block_link_change == 0) {
						if(set_if_status(ETH_PORT(i),IF_DOWN) == -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
						if(set_if_change_status(ETH_PORT(i),IF_STATUS_CHANGE) == -1)
							DEBUG_ERR("%s: set ethernet port status change bits error!\n",__FUNCTION__);
					}
#endif		
			}
		}

		/************ Link up check ************/
		for(i=0; i<5; i++) {
			if ((REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) != 0) {
				
				if ((port_link_sts & (1<<i)) == 0) {	// the port which already linked up does not need to check ...

					/*=========== ###03 ===========*/
					// read DUT eee 100 ability
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
					rtl8651_setAsicEthernetPHYReg( i, 14, 0x3c );
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
					rtl8651_getAsicEthernetPHYReg( i, 14, &DUT_eee );

					// read Link partner eee 100 ability
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x7 );
					rtl8651_setAsicEthernetPHYReg( i, 14, 0x3d );
					rtl8651_setAsicEthernetPHYReg( i, 13, 0x4007 );
					rtl8651_getAsicEthernetPHYReg( i, 14, &Linkpartner_eee );

					if (  ( ((DUT_eee & 0x2) ) != 0)  && ( ((Linkpartner_eee & 0x2) ) != 0) )  {
						rtl8651_getAsicEthernetPHYReg( i, 21, &snr );
						snr = snr | 0x4000;
						rtl8651_setAsicEthernetPHYReg( i, 21, snr );
					}

					if ( ((Linkpartner_eee & 0x2) ) != 0)
						port_linkpartner_eee |= (1 << i);						
					else
						port_linkpartner_eee &= ~(1 << i);						
					
					/*=========== ###02 ===========*/
					/*
					  1.      reg17 = 0x1f10!Aread reg29, for SNR
					  2.      reg17 =  0x1f11!Aread reg29, for AGC
					  3.      reg17 = 0x1f18!Aread reg29, for cb0
					 */					
					// 1. for SNR
					snr = 0;
					for(val=0; val<3; val++) {
						rtl8651_getAsicEthernetPHYReg(i, 29, &agc);
						snr += agc;
					}
					snr = snr / 3;
				
					// 3. for cb0
					rtl8651_getAsicEthernetPHYReg( i, 17, &val );
					val = (val & 0xfff0) | 0x8;
					rtl8651_setAsicEthernetPHYReg( i, 17, val );					
					rtl8651_getAsicEthernetPHYReg( i, 29, &cb0 );

					// 2. for AGC
					val = (val & 0xfff0) | 0x1;
					rtl8651_setAsicEthernetPHYReg( i, 17, val );
					rtl8651_getAsicEthernetPHYReg( i, 29, &agc );

					// set bit 3~0 to 0x0 for reg 17
					val = val & 0xfff0;
					rtl8651_setAsicEthernetPHYReg( i, 17, val );

					if ( ( (    ((agc & 0x70) >> 4) < 4    ) && ((cb0 & 0x80) != 0) ) || (snr > 4150) ) { // "> 4150" = "< 18dB"
						rtl8651_restartAsicEthernetPHYNway(i+1, i);				
					}
#if defined(CONFIG_RTL_REPORT_LINK_STATUS)
					if(block_link_change == 0) {
						if(set_if_status(ETH_PORT(i),IF_UP) == -1)
							DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
						if(set_if_change_status(ETH_PORT(i),IF_STATUS_CHANGE) == -1)
							DEBUG_ERR("%s :set ethernet port status change bits error!\n",__FUNCTION__);
					}
#endif	
				}
				new_port_link_sts = new_port_link_sts | (1 << i);
			}
		}	
		port_link_sts = new_port_link_sts;
		
    }
#endif
#endif
	if (status & (RX_DONE_IP_ALL |PKTHDR_DESC_RUNOUT_IP_ALL)) {
		*(volatile unsigned int *)(CPUIIMR) &= ~(RX_DONE_IE_ALL | PKTHDR_DESC_RUNOUT_IE_ALL);
#ifdef ISR_DIRECT
		interrupt_dsr_rx((unsigned long)cp);
#else
		call_dsr = 1;
#endif
	}

	if (status &TX_ALL_DONE_IP_ALL) {
		*(volatile unsigned int *)(CPUIIMR) &= ~TX_ALL_DONE_IP_ALL;
#ifdef TX_ISR_DIRECT
		interrupt_dsr_tx((unsigned long)cp);
#else
		call_dsr = 1;
#endif
	}

	cp->isr_status |= status;

	return call_dsr;
}

//----------------------------------------------------------------------------
static void interrupt_dsr(struct net_device *dev)
{
	struct re_private *cp = dev->priv;

	if (!cp->isr_status)
		interrupt_isr(dev);

#ifndef ISR_DIRECT		
	if (cp->isr_status & (RX_DONE_IP_ALL |PKTHDR_DESC_RUNOUT_IP_ALL)) {
		//diag_printf("dsr_Rx cp=%p\n", cp);
		interrupt_dsr_rx((unsigned long)cp);
	}
#endif

#ifndef TX_ISR_DIRECT
	if (cp->isr_status & TX_ALL_DONE_IP_ALL) {
		interrupt_dsr_tx((unsigned long)cp);
	}
#endif

	cp->isr_status = 0;

}

//----------------------------------------------------------------------------
static int can_xmit(struct net_device *dev)
{
	return can_send();
}

#if 1//for fpdebug
unsigned int statistic_total;
unsigned int statistic_ps;
unsigned int statistic_fp;
unsigned int statistic_skb_fp;
#endif

void rtl865x_stats(int clear)
{
	struct net_device *dev = reNet[0];
	struct re_private *cp = dev->priv;
	Rltk819x_t *info = dev->info;;

	if (clear) {
		diag_printf("clear Ethernet statistics\n");
		memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		reused_skb_num = 0;
#endif
#if 1//for fpdebug
		statistic_total = 0;
		statistic_ps = 0;
		statistic_fp= 0;
		statistic_skb_fp = 0;
#endif	

		return;
	}

	diag_printf("  Statistics...\n");
	diag_printf("    interface:    %s\n", cp->dev->name);
	diag_printf("    tx_packets:   %lu\n", cp->net_stats.tx_packets);
	diag_printf("    tx_bytes:     %lu\n", cp->net_stats.tx_bytes);
	diag_printf("    tx_fails:     %lu\n", cp->net_stats.tx_errors);
	diag_printf("    tx_drops:     %lu\n", cp->net_stats.tx_dropped);
	diag_printf("    rx_packets:   %lu\n", cp->net_stats.rx_packets);
	diag_printf("    rx_bytes:     %lu\n", cp->net_stats.rx_bytes);

	if (NEXT_DEV(cp) && NEXT_CP(cp)->opened) {
		cp = NEXT_CP(cp);
		diag_printf("\n");
		diag_printf("    interface:    %s\n", cp->dev->name);
		diag_printf("    tx_packets:   %lu\n", cp->net_stats.tx_packets);
		diag_printf("    tx_bytes:     %lu\n", cp->net_stats.tx_bytes);
		diag_printf("    tx_fails:     %lu\n", cp->net_stats.tx_errors);
		diag_printf("    tx_drops:     %lu\n", cp->net_stats.tx_dropped);
		diag_printf("    rx_packets:   %lu\n", cp->net_stats.rx_packets);
		diag_printf("    rx_bytes:     %lu\n", cp->net_stats.rx_bytes);
	}
	diag_printf("\n");
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	diag_printf("    skb_free_num: %lu\n", (LUINT) eth_skb_free_num+rx_skb_queue.qlen);
	diag_printf("    reused_cnt:   %lu\n", (LUINT)reused_skb_num);
#else
	diag_printf("    skb_free_num: %u\n", rx_skb_queue.qlen);
#endif
#ifdef TX_PKT_FREE_QUEUE
	diag_printf("    tx_queue: %u\n", info->tx_queue.qlen);
#endif
#ifdef CONFIG_RTK_CHECK_ETH_WAN_PORT_RX_HANG	
	{
		unsigned int port;
#if defined(CONFIG_RTL_8211E_SUPPORT)
		for (port=1; port<MAX_PORT_NUMBER; port++)
#else
		for (port=0; port<MAX_PORT_NUMBER; port++)
#endif
		{
			diag_printf("  rx_hang[%d]: %u\n", eth_port_rx_hang_count[port]);
			diag_printf("	 AN1tocbcount[%d]: %u\n", AN1tocbcount[port]);
			diag_printf("	 AN2tosnrcount[%d]: %u\n", AN2tosnrcount[port]);
			diag_printf("	 AN3tosnrcount[%d]: %u\n", AN3tosnrcount[port]);
		}
	}
#endif

	#if 1//for fpdebug
	diag_printf("  statistic_total %u\n", statistic_total);
	diag_printf("  statistic_ps %u\n", statistic_ps);
	diag_printf("  statistic_fp  %u\n", statistic_fp);
	diag_printf("  statistic_skb_fp %u\n", statistic_skb_fp);
	#endif
	diag_printf("\n");
}

#endif /* __ECOS */

//----------------------------------------------------------------------------

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
static int bridge_wan_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device *wan_dev = NULL;
	wan_dev = get_wan_from_vlan();
	//skb->dev = wan_dev;
	//printk("Virtual eth:%02x bridge packet to WAN:%02x\n",dev,wan_dev);
	wan_dev->hard_start_xmit(skb,wan_dev);
	return 0;
}
#endif //CONFIG_RTK_VLAN_ROUTETYPE

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
static inline int rtl_process_passthru_tx(struct sk_buff *skb)
{
	struct net_device *dev;
	//struct sk_buff *skb = NULL;
 	struct re_private *cp;

	if(old_passThru_flag)
	{
		dev = skb->dev;
		if (dev==pDev)
		{
			if (FAILED!=rtl_isEthPassthruFrame(skb->data))
			{
				cp = pDev->priv;
				dev=skb->dev=cp->dev;
				//diag_printf("PASSTHROUGH,DEV:%s,[%s]:[%d].\n",dev->name,__FUNCTION__,__LINE__);
			}
			else
			{
				dev_kfree_skb_any(skb);
				return FAILED;
			}
		}
	}

	return SUCCESS;
}
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)
int re865x_setMCastTxPortlist(struct sk_buff *skb, struct net_device *dev, unsigned int *mCastPortlist)
{
	int32 ret;
	 struct re_private *cp;
	struct iphdr *iph=NULL;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct ip6_hdr *ipv6h=NULL;
	#endif
	unsigned int l4Protocol=0;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	//unsigned int nicTxportlist;
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct sk_buff *skb_wan=NULL;
	rtl_nicTx_info	nicTx_wan;
#endif
	
	if((skb==NULL) || (dev==NULL) )
	{
		return -1;
	}

	if((igmpsnoopenabled==0) || (nicIgmpModuleIndex==0xFFFFFFFF))
	{
		return -1;
	}


	cp = dev->priv;

	*mCastPortlist=cp->port & ((1<<RTL8651_MAC_NUMBER)-1);
	
	if((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
	{
		
		iph = re865x_getIpv4Header(skb->data);
		if(iph)
		{
			
			l4Protocol=iph->protocol;
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP) )
			{
				
				/*if(cp->portnum<=1)
				{
					return -1;
				}*/
				
				/*only process tcp/udp in igmp snooping data plane*/
				multicastDataInfo.ipVersion=4;
				//memcpy(multicastDataInfo.sourceIp,&(iph->saddr),4);
				//memcpy(multicastDataInfo.groupAddr,&(iph->daddr),4);
				
				multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
				multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);

				multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
				multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
				
				ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				//diag_printf("multicastFwdInfo.fwdPortMask:%x,iph->saddr:%x,iph->daddr:%x,[%s]:[%d].\n",multicastFwdInfo.fwdPortMask,iph->saddr,iph->daddr,__FUNCTION__,__LINE__);
				*mCastPortlist =(*mCastPortlist)& multicastFwdInfo.fwdPortMask;



			}
            #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
			//fix tim; upnp
                if( vlan_enable && vlan_bridge_enable && vlan_bridge_multicast_enable)
				{
					if((cp->id == vlan_bridge_tag) && (nicTx->portlist & RTL_WANPORT_MASK))
					{
						nicTx->portlist &= (~RTL_WANPORT_MASK);
						skb_wan = skb_copy(skb, GFP_ATOMIC);
						 if(skb_wan!=NULL)
					     {
					       	nicTx_wan.txIdx=0;
                            nicTx_wan.vid = vlan_bridge_multicast_tag;
							nicTx_wan.portlist = RTL_WANPORT_MASK;
							nicTx_wan.srcExtPort = 0;
							nicTx_wan.flags = (PKTHDR_USED|PKT_OUTGOING);
							_dma_cache_wback_inv((unsigned long) skb_wan->data, skb_wan->len);
							if (RTL_swNic_send((void *)skb_wan, skb_wan->data, skb_wan->len, &nicTx_wan) < 0)
							{
								dev_kfree_skb_any(skb_wan);
							}
						}
			}

					}//hw-vlan
			#endif

		}
	}
#if defined (CONFIG_RTL_MLD_SNOOPING)
	else if ((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xff))
	{

		if(mldSnoopEnabled!=TRUE)
		{
			return -1;
		}
		/*if(cp->portnum<=1)
		{
			return -1;
		}*/

		ipv6h=re865x_getIpv6Header(skb->data);
		if(ipv6h!=NULL)
		{
			l4Protocol=re865x_getIpv6TransportProtocol(ipv6h);
			/*udp or tcp packet*/
			if((l4Protocol==IPPROTO_UDP) || (l4Protocol==IPPROTO_TCP))
			{
					/*only process tcp/udp in igmp snooping data plane*/
				multicastDataInfo.ipVersion=6;
				memcpy(&multicastDataInfo.sourceIp, &ipv6h->ip6_src, 16);
				memcpy(&multicastDataInfo.groupAddr, &ipv6h->ip6_dst, 16);

				multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
				multicastDataInfo.sourceIp[1] = ntohl(multicastDataInfo.sourceIp[1]);
				multicastDataInfo.sourceIp[2] = ntohl(multicastDataInfo.sourceIp[2]);
				multicastDataInfo.sourceIp[3] = ntohl(multicastDataInfo.sourceIp[3]);
				multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);
				multicastDataInfo.groupAddr[1] = ntohl(multicastDataInfo.groupAddr[1]);
				multicastDataInfo.groupAddr[2] = ntohl(multicastDataInfo.groupAddr[2]);
				multicastDataInfo.groupAddr[3] = ntohl(multicastDataInfo.groupAddr[3]);
				
				ret= rtl_getMulticastDataFwdInfo(nicIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				*mCastPortlist =(*mCastPortlist)& multicastFwdInfo.fwdPortMask;

			}

		}


	}
#endif
	return 0;
}
#endif

__IRAM_SECTION_
__MIPS16
static int rtl865x_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
//	unsigned long flags;
 	struct re_private *cp = dev->priv;
 #if defined(CONFIG_RTL8196_RTL8366) ||defined(CONFIG_RTL_VLAN_SUPPORT)	
 	struct sk_buff *newskb = NULL;
 #endif	
 #if defined (CONFIG_RTL_IGMP_SNOOPING)
 	unsigned int mCastPortlist;
#endif
	unsigned int txPortlist;
/*prevent boardcast packet turn round */
#ifdef PREVENT_BCAST_REBOUND	
	unsigned  char *DA ;
	struct bcast_tr_s *bacst_Ptr = &bcast ;
#endif
#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	int ret_check = FAILED;
#endif 

#if defined (CONFIG_RTL_PPPOE_DIRECT_REPLY)
	extern int magicNum;
	if(magicNum == -1){	
		extract_magicNum(skb);
	}
#endif

#if !defined(CONFIG_RTL8197B_PANA) && !defined(CONFIG_RTL865X_PANAHOST)
	if (!reNet[0] || !((struct re_private *)reNet[0]->priv)->opened) {
		cp->net_stats.tx_dropped++;
		dev_kfree_skb_any(skb);		
		return 0;	
	}
#endif
	#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	ret_check = rtl_qos_rate_limite_check(RTL_TX_DIR, dev->name, skb);
	if (ret_check ==  SUCCESS){
		// download traffic control of wired, added by zhuhuan on 2016.03.01
		dev_kfree_skb_any(skb);		
		return 0;
	}
	else{
		
	}
	#endif
#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_netsniper_check(skb, dev);
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	
	int retval = FAILED;
	retval = rtl_process_passthru_tx(skb);
	if(FAILED == retval)
		return retval;
	if(dev==pDev)
	{
		dev=skb->dev;
		cp = dev->priv;
	}
	
#endif
	txPortlist = cp->port;
	if(skb->data[0]&0x1)
	{
		//printk("vid:%d,cp->portmask:%x,[%s]:[%d].\n",cp->id,cp->portmask,__FUNCTION__,__LINE__);
		/*multicast process*/
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		/*multicast process*/
		if(	((skb->data[0]==0x01) && (skb->data[1]==0x00) && (skb->data[2]==0x5e))
#if defined (CONFIG_RTL_MLD_SNOOPING)
			||	((skb->data[0]==0x33) && (skb->data[1]==0x33) && (skb->data[2]!=0xFF))
#endif
		)
		{
			re865x_setMCastTxPortlist(skb, cp->dev, &mCastPortlist);
			
			txPortlist &= mCastPortlist;
			//diag_printf("txPortlist:%x,vid:%d,cp->port:%x,mCastPortlist:%x,[%s]:[%d].\n",txPortlist,cp->id,cp->port,mCastPortlist,__FUNCTION__,__LINE__);
		}
#endif
		
	}	
#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (cp->vlan_setting.global_vlan) {
		int vlan_decision = tx_vlan_process(dev, &cp->vlan_setting, skb, 0);
		if (vlan_decision) {
			cp->net_stats.tx_dropped++;
			dev_kfree_skb_any(skb);		
			return 0;				
		}
	}
#endif

#ifdef CONFIG_RTL_VLAN_SUPPORT
	if (rtl_vlan_support_enable) {
		if (skb_cloned(skb)){
			newskb = skb_copy(skb, GFP_ATOMIC);
			if (newskb == NULL)
			{
				cp->net_stats.tx_dropped++;
				dev_kfree_skb_any(skb);
				return 0;
			}
			dev_kfree_skb_any(skb);
			skb = newskb;
		}
		
		if (rtl_vlanEgressProcess(skb, dev->name, 0) < 0){
			cp->net_stats.tx_dropped++;
			dev_kfree_skb_any(skb);		
			return 0;
		}
	}
#endif

#ifdef	PREVENT_BCAST_REBOUND
	DA = skb->data;
	if(DA[0] & 0x01){

			//unsigned char* DDA;
			//panic_printk("\n====eth tx===>\n");			

			PBR_index %= P_BCAST_NUM;
			memcpy(&bacst_Ptr->entry[PBR_index].BCAST_SA , DA+6 ,6);
			bacst_Ptr->entry[PBR_index].time_stamp = jiffies ;
			//DDA = &bacst_Ptr->entry[PBR_index].BCAST_SA ;						
			PBR_index ++ ;
								
			//panic_printk("saved mac:%02X:%02X:%02X-%02X:%02X:%02X\n",
			//	DDA[0],DDA[1],DDA[2],DDA[3],DDA[4],DDA[5]);			
			//panic_printk("source mac:%02X:%02X:%02X-%02X:%02X:%02X\n",
			//	DA[6],DA[7],DA[8],DA[9],DA[10],DA[11]);

	}
#endif

#ifdef __KERNEL__
//	spin_lock_irqsave (&cp->lock, flags);
    dev->trans_start = jiffies;
#endif

#ifdef CONFIG_RTL8196_RTL8366
	if (*((unsigned short *)(skb->data+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q)) 
	{
		newskb = NULL;
		if (skb_cloned(skb)){
			newskb = skb_copy(skb, GFP_ATOMIC);
			if (newskb == NULL) {
				cp->net_stats.tx_dropped++;
				dev_kfree_skb_any(skb);		
				return 0;
			}
			dev_kfree_skb_any(skb);
			skb = newskb;
		}
			
		if (skb_headroom(skb) < 4 && skb_cow(skb, 4) !=0 ) {
			printk("%s-%d: error! (skb_headroom(skb) == %d < 4). Enlarge it!\n",
			__FUNCTION__, __LINE__, skb_headroom(skb));
			while (1) ;
		}
		skb_push(skb, VLAN_HLEN);

		memmove(skb->data, skb->data + VLAN_HLEN, 2 * VLAN_ETH_ALEN);
		*(uint16*)(&(skb->data[12])) = __constant_htons(ETH_P_8021Q);		/* TPID */
		*(uint16*)(&(skb->data[14])) = htons(cp->id);			/* VID */
		*(uint8*)(&(skb->data[14])) &= 0x0f;					/* clear most-significant nibble of byte 14 */
//		*(uint8*)(&(skb->data[14])) |= priorityField;

		skb->cb[9] = 1;
	}
#endif

#ifdef TX_SCATTER
	if (skb->list_num > 1) 
		skb->len = skb->total_len;
	else {
		skb->list_buf[0].buf = skb->data;		
		skb->list_buf[0].len = skb->len;
		skb->list_num = 1;
		skb->total_len = skb->len;
	}
#endif

    if (RTL_swNic_send((void *)skb, skb->data, skb->len, cp->vid, txPortlist) < 0) {	// tx queue full
		DEBUG_ERR("%s: tx failed!\n", dev->name);
		#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
			tx_ringFull_cnt++;
		#endif	
		netif_stop_queue(dev);

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
		if (dev == ((struct re_private *)reNet[1]->priv)->dev) {	// eth1 (WAN) be stopped, stop virtual interface which use eth1 tx function also
			netif_stop_queue(((struct re_private *)reNet[intf_ipv6_passthru]->priv)->dev);
		}
#endif

#ifdef CONFIG_RTK_VOIP
		// avoid packet lost!
		struct Qdisc *q=dev->qdisc;
		q->ops->requeue(skb, q);
		netif_schedule(dev);		
		return 0;
#else		
		cp->net_stats.tx_dropped++;
//		dev_kfree_skb_irq(skb);		
		dev_kfree_skb_any(skb);		

//		spin_unlock_irqrestore(&cp->lock, flags);		
		//return 1;
		/* tx queue full and drop this packet, return 0 to indicate the caller that the packet is done.
		 * return 1 the caller will resend it again and cause core dump (skb->list != NULL) in __kfree_skb().
		 */
		return 0;
#endif		
    }
	cp->net_stats.tx_packets++;		
	cp->net_stats.tx_bytes += skb->len;
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
		cp->tx_byte_cnt += skb->len;
#endif		

#ifdef BR_SHORTCUT
	pkt_cnt++;
#endif

//	spin_unlock_irqrestore(&cp->lock, flags);   
    return 0;
}

//---------------------------------------------------------------------------
static struct net_device_stats *rtl865x_get_stats(struct net_device *dev)
{
    struct re_private *cp = dev->priv;

#ifdef CONFIG_RTL865X_HW_TABLES		
	extern int gHwNatEnabled;

	if (gHwNatEnabled > 0) {
		int vid, port;
		uint32 val;
		
		for (vid=0; vid<2; vid++) {
			if (!memcmp(vlanSetting[vid].mac, dev->dev_addr, 6))
				break;
		}
		if (vid == 2) {
			//printk("%s: get statistics error!\n", __FUNCTION__);
			return NULL;
		}
		memset(&cp->net_stats,  '\0', sizeof(struct net_device_stats));

		if (!netif_running(dev))
			goto ret;
		for (port=0; port<MAX_PORT_NUMBER; port++) {	
			if 	(vlanSetting[vid].portmask & (1<<port)) {
				val = REG32(MIB_COUNTER_BASE+ 0x108 + 0x80*port);		
				if (val > 0) 
					cp->net_stats.rx_packets += val;

				val = REG32(MIB_COUNTER_BASE+ 0x13c + 0x80*port);	
				if (val > 0) 
					cp->net_stats.rx_packets += val;

				val = REG32(MIB_COUNTER_BASE+ 0x140 + 0x80*port);	
				if (val > 0) 
					cp->net_stats.rx_packets += val;
				
				val = REG32(MIB_COUNTER_BASE+ 0x100 + 0x80*port);
				if (val > 0) 
					cp->net_stats.rx_bytes += val;								

				val = REG32(MIB_COUNTER_BASE+ 0x808 + 0x80*port);		
				if (val > 0)
					cp->net_stats.tx_packets += val;

				val = REG32(MIB_COUNTER_BASE+ 0x80c + 0x80*port);		
				if (val > 0)
					cp->net_stats.tx_packets += val;
				
				val = REG32(MIB_COUNTER_BASE+ 0x810 + 0x80*port);		
				if (val > 0)
					cp->net_stats.tx_packets += val;

				val = REG32(MIB_COUNTER_BASE+ 0x800 + 0x80*port);		
				if (val > 0)
					cp->net_stats.tx_bytes += val;				
			}		
		}				
	}

ret:	
#endif

#ifdef CONFIG_RTL8196B_TLD
	if (cp->opened) {
		int i;

		cp->net_stats.collisions = 0;
		for (i=0; i<MAX_PORT_NUMBER; i++) {			
			if (cp->port & BIT(i)) {
				cp->net_stats.collisions += READ_MEM32(MIB_COUNTER_BASE+ OFFSET_ETHERSTATSCOLLISIONS_P0 + MIB_ADDROFFSETBYPORT*i);
			}			
		}
	}
#endif

    return &cp->net_stats;	
}

/*
#ifdef CONFIG_RTK_MESH
int check_LAN_linkstatus_any(void)
{
	unsigned int port_num=0;
	
#ifdef CONFIG_RTL8196_RTL8366
	int link_status=0;
    
	for (port_num = 0; port_num < 4; port_num++){
		rtl8366rb_getPHYLinkStatus (port_num, &link_status);//PHY4
		if (link_status==1)
			return 1;
	}

#else
	for (port_num = 1; port_num < 5; port_num++){
		if(READ_MEM32(PSRP0 + (port_num) * 4) & PortStatusLinkUp){	
			//printk("port %d plugged!!\n", port_num);
			return 1;
		}
		//printk("port %d unplug\n", port_num);
	}
#endif

	return 0;
}
#endif

*/

//---------------------------------------------------------------------------
static void rtl865x_stop_hw(struct net_device *dev, struct re_private *cp)
{
	swCore_down();
}
#ifdef RTL819X_PRIV_IOCTL_ENABLE
 int rtl819x_get_port_status(int portnum , struct lan_port_status *port_status)
{	
		uint32	regData;
		uint32	data0;

		if( portnum < 0 ||  portnum > 6)
			return -1;

		regData = READ_MEM32(PSRP0+((portnum)<<2));

		//printk("rtl819x_get_port_status port = %d data=%x\n", portnum,regData); //mark_debug
		data0 = regData & PortStatusLinkUp;		
		if (data0)
			port_status->link =1;
		else
			port_status->link =0;
				
		data0 = regData & PortStatusNWayEnable;
		if (data0)
			port_status->nway=1;
		else
			port_status->nway =0;

		data0 = regData & PortStatusDuplex;
		if (data0)
			port_status->duplex=1;
		else
			port_status->duplex =0;

		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		port_status->speed = data0 ; // 0 = 10M , 1= 100M , 2=1G ,

	       return 0;	
}

 int rtl819x_get_port_stats(int portnum , struct port_statistics *port_stats)
 {

	uint32 addrOffset_fromP0 =0;	

	//printk("rtl819x_get_port_stats port = %d \n", portnum); //mark_debug
	if( portnum < 0 ||  portnum > 6)
			return -1;

	addrOffset_fromP0 = portnum * MIB_ADDROFFSETBYPORT;
	
	//port_stats->rx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->rx_bytes =rtl8651_returnAsicCounter( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_mulpkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_bropkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_discard= rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) ;
	port_stats->rx_error= (rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
						
	//port_stats->tx_bytes =(uint32) (rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 )) ;
	port_stats->tx_bytes =rtl8651_returnAsicCounter( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_unipkts= rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_mulpkts= rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_bropkts= rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	port_stats->tx_discard= rtl8651_returnAsicCounter( OFFSET_IFOUTDISCARDS + addrOffset_fromP0 ) ;
	port_stats->tx_error= (rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) +
						 rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
	
	return 0;	
 }
 
int re865x_priv_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data_32;
	int portnum=0;
	struct lan_port_status port_status;
	struct port_statistics port_stats;

	data_32 = (unsigned long *)rq->ifr_data;
	if (copy_from_user(&portnum, data_32, 1*sizeof(unsigned long)))
	{
		return -EFAULT;
	}
	
	switch (cmd)
	{
	     case RTL819X_IOCTL_READ_PORT_STATUS:		 	
		 	rc = rtl819x_get_port_status(portnum,&port_status); //portnumber	
		 	if(rc != 0)
				return -EFAULT;
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_status, sizeof(struct lan_port_status)))
				return -EFAULT;
		 	break;
	     case RTL819X_IOCTL_READ_PORT_STATS:
		 	rc = rtl819x_get_port_stats(portnum,&port_stats); //portnumber	
		 	if(rc != 0)
				return -EFAULT;
		 	if (copy_to_user((void *)rq->ifr_data, (void *)&port_stats, sizeof(struct port_statistics)))
				return -EFAULT;
		 	break;	
	     default :
		 	rc = -EOPNOTSUPP;
			break;
	}	
	return SUCCESS;

}

#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_DRVETH
__NOMIPS16
#endif 
int re865x_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int32 rc = 0;
	unsigned long *data;
	int32 args[4];
	int32  * pRet;
	uint32 statCtrlReg, phyId = wan_port;
	unsigned int port_num=0;
	uint32 valBytes=0;
	#ifdef CONFIG_RTL_8367R_SUPPORT
	extern int rtk_port_phyStatus_get(uint32 port, uint32 *pLinkStatus, uint32 *pSpeed, uint32 *pDuplex);
	uint32 linkStatus, speed, duplex;
	#endif
    

	
	if (cmd != SIOCDEVPRIVATE)
	{
#ifndef RTL819X_PRIV_IOCTL_ENABLE
		goto normal;
 #else
 		rc = re865x_priv_ioctl(dev,rq,cmd); 
 		return rc;
#endif 
	}

	data = (unsigned long *)rq->ifr_data;

	if (copy_from_user(args, data, 4*sizeof(unsigned long)))
	{
		return -EFAULT;
	}

	switch (args[0])
	{
		case RTL8651_IOCTL_GETWANLINKSTATUS:
            pRet = (int32 *)args[3];
			*pRet = FAILED;
			rc = SUCCESS;
            #ifdef CONFIG_RTL8196_RTL8366
            {
                int link_status=0;
                /*Get link status of PHY 1 */
                rtl8366rb_getPHYLinkStatus (4, &link_status);//PHY4
		if(link_status)
                {
                    *pRet = SUCCESS;
                }
            }
		#elif defined(CONFIG_RTL_8367R_SUPPORT)
		{
			rc=rtk_port_phyStatus_get(RTL8367R_WAN, &linkStatus, &speed, &duplex);

			if(rc==SUCCESS)
			{
				if(linkStatus==1)
				{
					*pRet = SUCCESS;
				}
			}
		}
            #else
			if(READ_MEM32(PSRP0 + (wan_port) * 4) & PortStatusLinkUp){
				*pRet = SUCCESS;
			}
            #endif
			break;	
		case RTL8651_IOCTL_GETLANLINKSTATUS:
			pRet = (int32 *)args[3];
			
			rc = SUCCESS;
			memcpy(&port_num, pRet, 4);
            #if defined(CONFIG_RTL_8367R_SUPPORT)
            {
                rc=rtk_port_phyStatus_get(port_num, &linkStatus, &speed, &duplex);
            
                if(rc==SUCCESS)
                {
                    if(linkStatus==1)
                    {
                        *pRet = SUCCESS;
                    }
                    else
                    {
                        *pRet = FAILED;
                    }
                }
            }
            #else
			if(READ_MEM32(PSRP0 + (port_num) * 4) & PortStatusLinkUp){
				*pRet = SUCCESS;
			}else{
				*pRet = FAILED;
			}
            #endif
			break;			
/*
#ifdef CONFIG_RTK_MESH
		case RTL8651_IOCTL_GETLANLINKSTATUSALL:
			pRet = (int32 *)args[3];
			*pRet = FAILED;
			
			rc = SUCCESS;
			if (check_LAN_linkstatus_any()!=0)
				*pRet = SUCCESS;
			
			break;			
#endif
*/
		case RTL8651_IOCTL_GETLANLINKSPEED:
			pRet = (int32 *)args[3];
			rc = FAILED;
			memcpy(&port_num, pRet, 4);
            #ifdef CONFIG_RTL_8367R_SUPPORT
			{
				rc=rtk_port_phyStatus_get(port_num, &linkStatus, &speed, &duplex);

				if(rc==SUCCESS)
				{
					*pRet = speed;
				}				
			}
            #else
			switch(READ_MEM32(PSRP0 + (port_num) * 4) & PortStatusLinkSpeed_MASK)
			{
				case PortStatusLinkSpeed10M:
					*pRet = PortStatusLinkSpeed10M;
					rc = SUCCESS;
					break;
				case PortStatusLinkSpeed100M:
					*pRet = PortStatusLinkSpeed100M;
					rc = SUCCESS;
					break;
				case  PortStatusLinkSpeed1000M:
					*pRet = PortStatusLinkSpeed1000M;
					rc = SUCCESS;
					break;
				default:
					break;
			}
            #endif
			break;	
			
            case RTL8651_IOCTL_SETLANLINKSPEED:
                #if defined(CONFIG_RTL_8367R_SUPPORT)
                port_num = args[2];
                pRet = (int32 *)args[3];                
                if ((*pRet != HALF_DUPLEX_1000M))
                {
                    set_8367r_speed_mode(port_num, *pRet);
                }                
                #else
                /* 
                           *(int32 *)args[3]: 0: Auto, 0b1000: force to 10M, Auto, 0b1001: force to 100M, Auto, 0b1010: force to Giga, 
                           *0<-->PORT_AUTO, 0b1000<-->DUPLEX_10M, 0b1001<-->DUPLEX_100M, 0b1010<-->DUPLEX_1000M
                           */
            
                port_num = args[2];
                // bit23: 0:auto; 1:force
                // bit21:bit20 10M:00, 100M:01, 1000M:10
                //REG32(PCRP4) = (REG32(PCRP4) & (~0x00b00000) | (*pRet<<20));          
            
                phyId = port_num;
                pRet = (int32 *)args[3];
                rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg );
                statCtrlReg &= ~(CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
                if (*pRet == DUPLEX_10M)
                    statCtrlReg |= (CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
                else if (*pRet == DUPLEX_100M)
                    statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD);
                else
                    statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
                    
                rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg );
                rtl8651_getAsicEthernetPHYReg( phyId, 0, &statCtrlReg );
            
                statCtrlReg |= RESTART_AUTONEGO;
                rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg );             
                #endif
                rc = SUCCESS;
                break;
                
		case RTL8651_IOCTL_GETETHERLINKDUPLEX:
            pRet = (int32 *)args[3];
            rc = SUCCESS;
            memcpy(&port_num, pRet, 4);
            #if defined(CONFIG_RTL_8367R_SUPPORT)
            if ((rtk_port_phyStatus_get(port_num, &linkStatus, &speed, &duplex)) == 0) 
            {
                *pRet = duplex;
            }
            else
            {
                *pRet = FAILED;
            }
                
            #else
			//返回值有点问题，这里返回1表示全双工，0表示半双工，-1失败, llm
    		if(READ_MEM32(PSRP0 + (port_num) * 4) & PortStatusDuplex){	
    			*pRet = 1;	//llm modify, old: *pRet = SUCCESS;
    		}else{
    			*pRet = 0;	//llm modify, old: *pRet = FAILED;
    		}
            #endif
			break;	
			
        case RTL8651_IOCTL_SETETHERLINKDUPLEX:
            /*
                     *0:half duplex, 1:full duplex  
                     */
            port_num = args[2];
            pRet = (int32 *)args[3];
            //diag_printf("%s %d port_num=%d *pRet=%d\n", __FUNCTION__, __LINE__, port_num, *pRet);
            #if defined(CONFIG_RTL_8367R_SUPPORT)
            rtk_port_phyStatus_get(port_num, &linkStatus, &speed, &duplex);
            switch(speed)
            {
               case PortStatusLinkSpeed10M:
                    if (*pRet)
                    {
                        set_8367r_speed_mode(port_num, DUPLEX_10M);
                    }
                    else
                    {
                        set_8367r_speed_mode(port_num, HALF_DUPLEX_10M);
                    }
                   break;
               case PortStatusLinkSpeed100M:
                   if (*pRet)
                   {
                       set_8367r_speed_mode(port_num, DUPLEX_100M);
                   }
                   else
                   {
                       set_8367r_speed_mode(port_num, HALF_DUPLEX_100M);
                   }
                   break;
               case  PortStatusLinkSpeed1000M:
                   if (*pRet)
                   {
                       set_8367r_speed_mode(port_num, DUPLEX_1000M);
                   }
                   else
                   {
                       set_8367r_speed_mode(port_num, HALF_DUPLEX_1000M);
                   }
                   break;
               default:
                   break;
            }                
            #else                 
            phyId = port_num;
            rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg );
            switch(READ_MEM32(PSRP0 + (port_num) * 4) & PortStatusLinkSpeed_MASK)
            {
               case PortStatusLinkSpeed10M:
                    if (*pRet)
                    {
                        statCtrlReg |= (CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
                    }
                    else
                    {
                        statCtrlReg &= ~(CAPABLE_10BASE_TX_FD);
                    }
                   break;
               case PortStatusLinkSpeed100M:
                   if (*pRet)
                   {
                       statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD);
                   }
                   else
                   {
                       statCtrlReg &= ~(CAPABLE_100BASE_TX_FD);
                   }
                   break;
               default:
                   break;
            }                
            rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg );
            rtl8651_getAsicEthernetPHYReg( phyId, 0, &statCtrlReg );
            
            statCtrlReg |= RESTART_AUTONEGO;
            rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg );             
            #endif
            rc = SUCCESS;
            break;  
			
		case RTL8651_IOCTL_GETWANLINKSPEED:
			pRet = (int32 *)args[3];
			*pRet = FAILED;
			rc = FAILED;
			#ifdef CONFIG_RTL_8367R_SUPPORT
			{
				rc=rtk_port_phyStatus_get(RTL8367R_WAN, &linkStatus, &speed, &duplex);

				if(rc==SUCCESS)
				{
					*pRet = speed;
				}				
			}
			#else
			switch(READ_MEM32(PSRP0 + (wan_port) * 4) & PortStatusLinkSpeed_MASK)
			{
				case PortStatusLinkSpeed10M:
					*pRet = PortStatusLinkSpeed10M;
					rc = SUCCESS;
					break;
				case PortStatusLinkSpeed100M:
					*pRet = PortStatusLinkSpeed100M;
					rc = SUCCESS;
					break;
				case  PortStatusLinkSpeed1000M:
					*pRet = PortStatusLinkSpeed1000M;
					rc = SUCCESS;
					break;
				default:
					break;
			}
			#endif
			break;
		
		case RTL8651_IOCTL_SETWANLINKSPEED:
            #if defined(CONFIG_RTL_8367R_SUPPORT)
            phyId = wan_port;
			pRet = (int32 *)args[3];
            if ((*pRet != HALF_DUPLEX_1000M))
            {
                set_8367r_speed_mode(phyId, *pRet);
            }                
            #else
			/* 
				*(int32 *)args[3]: 0: Auto, 0b1000: force to 10M, Auto, 0b1001: force to 100M, Auto, 0b1010: force to Giga, 
			 */

			pRet = (int32 *)args[3];
			// bit23: 0:auto; 1:force
			// bit21:bit20 10M:00, 100M:01, 1000M:10
			//REG32(PCRP4) = (REG32(PCRP4) & (~0x00b00000) | (*pRet<<20));			

			if (wan_port == 5) /* 8211 */
				phyId = GIGA_P5_PHYID;
			
			rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg );
			statCtrlReg &= ~(CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
			//if (*pRet == 8)
			if (*pRet == DUPLEX_10M)
				statCtrlReg |= (CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
			//else if (*pRet == 9)
			else if (*pRet == DUPLEX_100M)
				statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD);
			else
				statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
				
			rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg );
			rtl8651_getAsicEthernetPHYReg( phyId, 0, &statCtrlReg );

			statCtrlReg |= RESTART_AUTONEGO;
			rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg );				
            #endif
			rc = SUCCESS;	
			break;
			
		/* llm add, for 8196e. 如果分别调用设置speed和duplex的ioctl设置wan口速度，中间需要
		   延时2s才行，因此写成一个ioctl */
		case TENDA_RTL8651_IOCTL_SETWANLINKSPEED:
			/* 
			 * args[1] not use
			 * args[2] speed
			 * args[3] duplex
			*/ 
			printf("%s():%d, set port %d %dM %s\n", __FUNCTION__, __LINE__,
				wan_port, args[2], args[3] ? "FULL" : "HALF");

			rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg );
			statCtrlReg &= ~(CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);

			if (args[2] == 10)	//10M
			{
				if(args[3] == 0)	//HALF
					statCtrlReg |= CAPABLE_10BASE_TX_HD;
				else	//FULL
					statCtrlReg |= (CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
			}
			else if (args[2] == 100)	//100M
			{
				if(args[3] == 0)	//HALF
					statCtrlReg |= CAPABLE_100BASE_TX_HD;
				else	//FULL
					statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD);
			}
			else	//auto
				statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
				
			rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg );
			rtl8651_getAsicEthernetPHYReg( phyId, 0, &statCtrlReg );

			statCtrlReg |= RESTART_AUTONEGO;
			rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg );				

			rc = SUCCESS;	
			break;
			
#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
		case RTL8651_IOCTL_GET_ETHER_EEE_STATE:
			pRet = (int32 *)args[3];
			*pRet = FAILED;
			rc = FAILED;						
						
			*pRet = READ_MEM32(0xbb804164);
			rc = SUCCESS;
			
			break;
#endif //#if defined(CONFIG_RTL8196C)
		
		case RTL8651_IOCTL_GET_ETHER_BYTES_COUNT:			
			pRet = (int32 *)args[3];			
			rc = FAILED;
			
			memcpy(&port_num, pRet, 4);
			*pRet = 0;

			valBytes = REG32(MIB_COUNTER_BASE+ 0x100 + 0x80*port_num);
			if (valBytes > 0) 
				*pRet += valBytes;								

			valBytes = REG32(MIB_COUNTER_BASE+ 0x800 + 0x80*port_num);		
			if (valBytes > 0)
				*pRet += valBytes;	

			rc = SUCCESS;
			
			break;	
			
#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
		case RTL8651_IOCTL_SET_IPV6_WAN_PORT: 
			pRet = (int32 *)args[1];
			
			memset(wan_if, 0x00, sizeof(wan_if));
			if ((int32)pRet == 1)
			{				
				strcpy(wan_if,rq->ifr_name);
			}
			break;
#endif			
		default:
			rc = SUCCESS;
			break;
	}

	return rc;
#ifndef RTL819X_PRIV_IOCTL_ENABLE			
normal:	
#endif
	switch (cmd)
        {
	    default:
		rc = -EOPNOTSUPP;
		break;
	}
	return rc;			

}

extern int32 free_pending_tx_skb(void);
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG
static int rtl865x_close(struct net_device *dev);
static int rtl865x_open(struct net_device *dev);
int check_tx_desc_hang(void);
static int tx_hang_cnt = 0;
#ifdef __KERNEL__
static struct net_device_stats tmp_net_stats[ETH_INTF_NUM];
#endif

static int if_opend[ETH_INTF_NUM];

static void reinit_eth_intf(void)
{
	uint32 flags;
	struct re_private *cp;
	int i;

	save_and_cli(flags);

#ifdef ETH_NEW_FC
	swCore_down(); // stop Tx/Rx ASAP
#endif
	for (i=0; i<ETH_INTF_NUM; i++) {
		if (reNet[i]) {
			cp = (struct re_private *)reNet[i]->priv;

			if_opend[i] = cp->opened;
			if (if_opend[i]) {
#ifdef __KERNEL__			
				memcpy(&tmp_net_stats[i], &cp->net_stats, sizeof(struct net_device_stats)); 
#endif
				rtl865x_close(reNet[i]); 
			}
		}
	}

#ifndef ETH_NEW_FC // move into rtl865x_close()
	free_pending_tx_skb(); 
#endif

	REG32(PCRP0) = REG32(PCRP0) & ~EnablePHYIf;	/* Disable PHY interface. */
	TOGGLE_BIT_IN_REG_TWICE(PCRP0, EnForceMode);
	REG32(PCRP1) = REG32(PCRP1) & ~EnablePHYIf;	/* Disable PHY interface. */
	TOGGLE_BIT_IN_REG_TWICE(PCRP1, EnForceMode);
	REG32(PCRP2) = REG32(PCRP2) & ~EnablePHYIf;	/* Disable PHY interface. */
	TOGGLE_BIT_IN_REG_TWICE(PCRP2, EnForceMode);
	REG32(PCRP3) = REG32(PCRP3) & ~EnablePHYIf;	/* Disable PHY interface. */
	TOGGLE_BIT_IN_REG_TWICE(PCRP3, EnForceMode);
	REG32(PCRP4) = REG32(PCRP4) & ~EnablePHYIf;	/* Disable PHY interface. */
	TOGGLE_BIT_IN_REG_TWICE(PCRP4, EnForceMode);

	/* Queue Reset Register */
	REG32(QRR) = QRST;
	mdelay(10);        
	REG32(QRR) = 0;
	mdelay(10);        

	REG32(SSIR) = 0;
	mdelay(10);        
	REG32(SSIR) = SwitchSemiRst;
	mdelay(10);        
	REG32(SSIR) = TRXRDY;
	mdelay(10);        
		   
	for (i=0; i<ETH_INTF_NUM; i++) {

		if (reNet[i] && if_opend[i]) {
			rtl865x_open(reNet[i]); 
			cp = (struct re_private *)reNet[i]->priv;
#ifdef __KERNEL__			
			memcpy(&cp->net_stats, &tmp_net_stats[i], sizeof(struct net_device_stats)); 
#endif
		}
	}

	//REG32(GISR) = LX0_BFRAME_IP;		// write clear for bit 1 "LBC 0 bus frame time-out interrupt pending flag"

	tx_hang_cnt++;
	restore_flags(flags);
}
#endif

#ifdef CONFIG_RTL_8196E
void	refine_phy_setting(void)
{
	int i, start_port = 0;
	uint32 val;

	val = (REG32(BOND_OPTION) & BOND_ID_MASK);
	
	if ((val == BOND_8196ES) || (val == BOND_8196ES1) || (val == BOND_8196ES2) || (val == BOND_8196ES3))
		return;
	
	if ((val == BOND_8196EU) || (val == BOND_8196EU1) || (val == BOND_8196EU2) || (val == BOND_8196EU3))
		start_port = 4;
	
	for (i=start_port; i<5; i++) {
		rtl8651_setAsicEthernetPHYReg( i, 25, 0x6964);
		rtl8651_getAsicEthernetPHYReg(i, 26, &val);
		rtl8651_setAsicEthernetPHYReg(i, 26, ((val & (0xff00)) | 0x9E) );

		rtl8651_getAsicEthernetPHYReg(i, 17, &val);
		rtl8651_setAsicEthernetPHYReg( i, 17, ((val & (0xfff0)) | 0x8)  );

		rtl8651_getAsicEthernetPHYReg( i, 29, &val );
		if ((val & 0x8080) == 0x8080) {
			rtl8651_getAsicEthernetPHYReg( i, 21, &val );
			rtl8651_setAsicEthernetPHYReg( i, 21, (val  | 0x8000) );
			rtl8651_setAsicEthernetPHYReg( i, 21, (val & ~0x8000) );
		}
	}	
	return;
}
#endif

//---------------------------------------------------------------------------
#ifdef CONFIG_RTK_CHECK_ETH_WAN_PORT_RX_HANG	
static unsigned long long pre_asic_rx_bytes[MAX_PORT_NUMBER]={0};
static unsigned long long pre_asic_tx_bytes[MAX_PORT_NUMBER]={0};
static unsigned int count_no_rx[MAX_PORT_NUMBER]={0};
static unsigned int count_tx_increase[MAX_PORT_NUMBER]={0};
static unsigned int count_no_tx[MAX_PORT_NUMBER]={0};
static int rtl_wan_connect_flag;
unsigned int set_wan_connect_flag(int flag)
{
	rtl_wan_connect_flag=flag;
}
unsigned long long rtl865x_get_trx_bytes(int portid, int direction)
{
	uint32 addrOffset_fromP0 = portid * MIB_ADDROFFSETBYPORT;
	if(portid > RTL8651_PORT_NUMBER)
		return 0;
	if(0 == direction)
		return rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0);
	if(1 == direction)
		return rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);
	return 0;
}
int check_eth_port_rx_hang(unsigned int port)
{
	unsigned long long asic_rx_bytes;	
	unsigned long long asic_tx_bytes;
	
//	if(0 == rtl_wan_connect_flag)
//	{
//		return 0;
//	}
	if((REG32(PSRP0 + (port * 4)) & PortStatusLinkUp) == 0)
		return 2;
 	asic_rx_bytes=rtl865x_get_trx_bytes(port,0);
	asic_tx_bytes=rtl865x_get_trx_bytes(port,1);
	if(asic_rx_bytes == pre_asic_rx_bytes[port])
	{
		count_no_rx[port]++;
		if(asic_tx_bytes == pre_asic_tx_bytes[port])
		{
			count_no_tx[port]++;
		}
		else
		{
			count_tx_increase[port]++;
		}
	} else
	{
		count_no_rx[port]=0;
		count_tx_increase[port]=0;
	}
	pre_asic_rx_bytes[port]=asic_rx_bytes;
	pre_asic_tx_bytes[port]=asic_tx_bytes;
	if((count_no_rx[port] > 20) && (count_tx_increase[port] >1)) {
		count_no_rx[port]=0;
		count_tx_increase[port]=0;
		return 1;
	}	
	return 0;
}
#if 0
int rtl_wan_port_on_off()
{
	rtl_setPhyPowerOff(wan_port);
	rtl_setPhyPowerOn(wan_port);
}
#endif
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#ifndef CONFIG_RTL_8367R_SUPPORT
static unsigned int AutoDownSpeed_10M[MAX_PORT_NUMBER];
static unsigned int DownSpeed_counter[MAX_PORT_NUMBER];
static unsigned int ReverSpeed_flag[MAX_PORT_NUMBER];
static unsigned int match_count[MAX_PORT_NUMBER];

//static unsigned int timer_checkPhyCbSnCount=0;

static int rtl819xD_checkPhyCbSnr(void)
{
	int ret = 0;
	unsigned int port=0;
	int  curr_sts=0;
	int link_speed_10M=0;
	unsigned int  val=0, cb=0, snr=0, cache_lpi=0;
	unsigned char cb_low=0, cb_high=0;
	int cb_flag[MAX_PORT_NUMBER];
	int snr_flag[MAX_PORT_NUMBER];
	
	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
#if defined(CONFIG_RTL_8211E_SUPPORT)
	for (port=1; port<MAX_PORT_NUMBER; port++)
#else
	for (port=0; port<MAX_PORT_NUMBER; port++)
#endif
	{
		//diag_printf("%s %d port %d\n",__FUNCTION__,__LINE__,port);

#ifdef CONFIG_RTK_CHECK_ETH_WAN_PORT_RX_HANG	
		/*jwj: check all ports rx now*/
		if (ret = check_eth_port_rx_hang(port)) {
			if (ret == 1)
				eth_port_rx_hang_count[port]++;
			//rtl_wan_port_on_off();
		} else {
			continue;
		}
#endif
		cb_flag[port] = 0;
		snr_flag[port]= 0;
	
		curr_sts = (REG32(PSRP0 + (port * 4)) & PortStatusLinkUp) >> 4;
		if ((REG32(PSRP0 + (port * 4)) & PortStatusLinkSpeed_MASK) == PortStatusLinkSpeed10M)
			link_speed_10M= 1;	
		else
			link_speed_10M= 0;	
		if(AutoDownSpeed_10M[port]==0x12345678)
		{
		   DownSpeed_counter[port]=DownSpeed_counter[port]+1;
			if((!curr_sts)&&(ReverSpeed_flag[port]==1))
			{
				REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|((NwayAbility1000MF|NwayAbility100MF|NwayAbility100MH)));
				DownSpeed_counter[port]=0;
				AutoDownSpeed_10M[port]=0;
				ReverSpeed_flag[port]=0;
				match_count[port]=0;
#ifdef CONFIG_RTL_REPORT_LINK_STATUS		
				block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
				rtl8651_restartAsicEthernetPHYNway(port+1,port);
			}
			if((!curr_sts)&&(DownSpeed_counter[port]>5))
			{
					REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))|((NwayAbility1000MF|NwayAbility100MF|NwayAbility100MH)));
					DownSpeed_counter[port]=0;
					AutoDownSpeed_10M[port]=0;
					ReverSpeed_flag[port]=0;
					match_count[port]=0;
#ifdef CONFIG_RTL_REPORT_LINK_STATUS					
					block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
					rtl8651_restartAsicEthernetPHYNway(port+1,port);
			}
			else if(curr_sts&&(DownSpeed_counter[port]<5))
			{
				ReverSpeed_flag[port]=1;
			}
		}
		else
		{
			AutoDownSpeed_10M[port]=0;
			DownSpeed_counter[port]=0;
			ReverSpeed_flag[port]=0;
		}
		if ( (curr_sts == 1)&&(!link_speed_10M)) 
		{
						rtl8651_setAsicEthernetPHYReg( port, 25, (0x6964) );
						rtl8651_getAsicEthernetPHYReg(port, 26, &val );
						rtl8651_setAsicEthernetPHYReg( port, 26, ((val&0xBF00)|0x9E) ); // Close new_SD.
						rtl8651_getAsicEthernetPHYReg(port, 17, &val );
						rtl8651_setAsicEthernetPHYReg( port, 17, ((val&0xFFF0)|0x8) );  //CB bit 
						rtl8651_getAsicEthernetPHYReg( port, 29, &cb );
						cb_low = cb&0xff;
						cb_high = (cb&0xff00)>>8;
						if ((((cb&(1<<15))>>15) ||(cb_high<=5)) && (((cb&(1<<7))>>7) ||(cb_low<=5))) {

							cb_flag[port] = 1;
							match_count[port]++;
#ifdef CONFIG_RTL_REPORT_LINK_STATUS							
							block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
//							REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~(NwayAbility1000MF|NwayAbility100MF|NwayAbility100MH)));
							rtl8651_restartAsicEthernetPHYNway(port+1,port);
							AN1tocbcount[port]++;
							diag_printf("AN1-->cb\r\n" );
						}
						rtl8651_setAsicEthernetPHYReg( port, 25, (0x6964) );
						rtl8651_getAsicEthernetPHYReg(port, 26, &val );
						rtl8651_setAsicEthernetPHYReg( port, 26, ((val&0xBF00)|0x9E) ); // Close new_SD.
						rtl8651_getAsicEthernetPHYReg( port, 17, &val );
						rtl8651_setAsicEthernetPHYReg( port, 17, ((val&0xFFF0)) );//SNR bit
						rtl8651_getAsicEthernetPHYReg( port, 29, &snr );
						if (snr > 0x1000) {
							if (cb_flag[port] == 0) {
								snr_flag[port] = 1;
								match_count[port]++;
#ifdef CONFIG_RTL_REPORT_LINK_STATUS							
								block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
								rtl8651_restartAsicEthernetPHYNway(port+1,port);
								AN2tosnrcount[port]++;
								diag_printf("AN2-->snr\r\n" );
							}
						}
						rtl8651_setAsicEthernetPHYReg( port, 25, (0x2040) );
						rtl8651_getAsicEthernetPHYReg(port, 26, &val );
						rtl8651_setAsicEthernetPHYReg( port, 26, ((val&0xBF00)|0x0c) ); // Close new_SD.
						rtl8651_setAsicEthernetPHYReg(port, 31, 4);
						rtl8651_getAsicEthernetPHYReg(port, 25, &val );
						rtl8651_setAsicEthernetPHYReg(port, 25, ((val&0xf) |0x3)); 
						rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
						rtl8651_getAsicEthernetPHYReg( port, 29, &cache_lpi );
						if (cache_lpi & 0x20) {
							if ((cb_flag[port]==0) && (snr_flag[port]==0)) {
								match_count[port]++;
#ifdef CONFIG_RTL_REPORT_LINK_STATUS
								block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
								rtl8651_restartAsicEthernetPHYNway(port+1,port);
							    	AN3tosnrcount[port]++;
								diag_printf("AN3-->cache_lpi\r\n" );
							}
						}

						if (match_count[port] >= 5) {
							REG32(PCRP0+(port<<2))= ((REG32(PCRP0+(port<<2)))&(~(NwayAbility1000MF|NwayAbility100MF|NwayAbility100MH)));
#ifdef CONFIG_RTL_REPORT_LINK_STATUS
							block_link_change=RTK_BLOCK_LINK_CHANGE_PERIOD;
#endif
							rtl8651_restartAsicEthernetPHYNway(port+1,port);
							AutoDownSpeed_10M[port]=0x12345678;
							DownSpeed_counter[port]=0;
							ReverSpeed_flag[port]=0;
							match_count[port] = 0;
							diag_printf("Down port speed to 10M!\r\n" );
						}
		}
	}
	return 0;
}
#endif
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT)
void rtl_check_8367r_link_status()
{
		extern int rtk_int_status_get(rtk_int_8367r_status_t* pStatusMask);
		extern int rtk_int_status_set(rtk_int_8367r_status_t pStatusMask);
	
		int i;
		rtk_int_8367r_status_t status_8367r;
		memset(&status_8367r,0,sizeof(rtk_int_8367r_status_t));
	
		if(rtk_int_status_get(&status_8367r) == 0){
			if(status_8367r.value[0] & 1<<0 != 0){
				newLinkPortMask = rtl865x_getPhysicalPortLinkStatus();
				
#ifdef CONFIG_RTL_8197D_DYN_THR				
				rtl819x_setQosThreshold(curLinkPortMask, newLinkPortMask);
#endif
		

#if defined(CONFIG_RTL_REPORT_LINK_STATUS)

			if(block_link_change ==0) {
				for(i=0; i<5; i++) {
					if((curLinkPortMask & (1<<i))==0){
						if ((newLinkPortMask & (1<<i))	!= 0)	
							if(set_if_status(ETH_PORT(i),IF_UP) ==	-1)
								DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
					}
					if((curLinkPortMask & (1<<i))!=0){
						if ((newLinkPortMask & (1<<i)) == 0)	
							if(set_if_status(ETH_PORT(i),IF_DOWN) == -1)
								DEBUG_ERR("%s: set ethernet port status bits error!\n",__FUNCTION__);
					}
	
					if((curLinkPortMask & (1<<i)) != (newLinkPortMask & (1<<i))){
						//printf("\n%s:%d calling set_if_change_status()\n", __FUNCTION__,__LINE__);
						if(set_if_change_status(ETH_PORT(i),IF_STATUS_CHANGE) == -1)
							DEBUG_ERR("%s: set ethernet port status change bit error!\n",__FUNCTION__);
					}
				}
			}	
#endif
	
#ifdef RTK_SYSLOG_SUPPORT
				for(i=0; i<5; i++) {
					if((curLinkPortMask & (1<<i))==0){
						if ((newLinkPortMask & (1<<i)) != 0)	
							printk("Ethernet port%d link up\n",i);
					}
					if((curLinkPortMask & (1<<i))!=0){
						if ((newLinkPortMask & (1<<i)) == 0)
							printk("Ethernet port%d link down\n",i);
					}
				}
#endif		
				curLinkPortMask=newLinkPortMask;
	
				status_8367r.value[0] = 0x1;
				status_8367r.value[1] = 0;
				rtk_int_status_set(status_8367r);
				
			}
		}
}
#endif

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT) && defined(CONFIG_RTL819X_GPIO)
int rp;
unsigned int regData, regData1, regData2, prelink;
extern int rtl_mdio_read(unsigned int mdio_phyaddr,unsigned int reg, unsigned int *pdata);
int extphyid = 0x5;
#define BIT10F 	(1<<6)
#define BIT100H 	(1<<7)
#define BIT100F 	(1<<8)
	
static void ckeck_link_speed(void){

	for (rp = 0; rp < 2; rp++)
		rtl_mdio_read(extphyid, 1, &regData);

	if (regData & (1<<2) && !(prelink & (1<<2)) ){

		for (rp = 0; rp < 2; rp++)
			rtl_mdio_read(extphyid, 9, &regData1);
		
		for (rp = 0; rp < 2; rp++)
			rtl_mdio_read(extphyid, 10, &regData2);

		if ( ((regData1>>8) & (regData2>>10)) & (1<<1)) { //force Giga
			REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
			| (EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex);
		} else {
			for (rp = 0; rp < 2; rp++)
				rtl_mdio_read(extphyid, 4, &regData1);
		
			for (rp = 0; rp < 2; rp++)
				rtl_mdio_read(extphyid, 5, &regData2);

			if (regData1 & regData2 & BIT100H) { //100M
				if (regData1 & regData2 & BIT100F){
					REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
					| (EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex); //100F
				}
				else{
					REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
					| (EnForceMode| ForceLink|ForceSpeed100M); //100H		
				}
			} else { //10M
				if (regData1 & regData2 & BIT10F){
					REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
					| (EnForceMode| ForceLink|ForceSpeed10M |ForceDuplex); //10F
				}
				else{
					REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
					| (EnForceMode| ForceLink|ForceSpeed10M); //10H
				}
			}
		}
	}

	prelink = regData;
}	

#endif


int total_time_for_5_port = 1500; //unit ms
int port_pwr_save_low = 0;

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(BR_SHORTCUT) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD)  || defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL8198) /*|| defined(CONFIG_RTK_MESH)*/ || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)  || defined(CONFIG_RTL_8197F)
static void one_sec_timer(unsigned long task_priv)
{
	uint32 flags;
	struct re_private *cp_next;
    	struct re_private *cp = ((struct net_device *)task_priv)->priv;

#ifdef ETH_NEW_FC
	if (!cp->opened)
		return;
#endif		
	cp_next = NEXT_CP(cp);
	//spin_lock_irqsave (&cp->lock, flags);
	save_and_cli(flags);

#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
	if (((struct net_device *)task_priv)->name[3] == '0' ) {	// eth0
		int port;

		for (port=0; port<RTL8651_PHY_NUMBER; port++) {
			update_mib_counter(port);		
			calculate_led_interval(port);
			if (led_cb[port].link_status & GPIO_LINK_STATE_CHANGE) {
				gpio_led_Interval_timeout(port);
			}
		}
	}	
#endif

#ifdef DYNAMIC_ADJUST_TASKLET
	if (((struct net_device *)task_priv)->name[3] == '0' && rx_pkt_thres > 0 && 
		((eth_flag&TASKLET_MASK) == 0))  { 
		if (rx_cnt > rx_pkt_thres) {
			if (!rx_tasklet_enabled) {
				rx_tasklet_enabled = 1;
			}							
		}               
		else {
			if (rx_tasklet_enabled) { // tasklet enabled
				rx_tasklet_enabled = 0;
			}						
		}       
		rx_cnt = 0;     
	}   
#endif
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
	//eth0
	cp->tx_avarage = (cp->tx_avarage/10)*7 + (cp->tx_byte_cnt/10)*3;
	if (cp->tx_avarage > cp->tx_peak)
		cp->tx_peak = cp->tx_avarage;
	cp->tx_byte_cnt = 0;
    
	cp->rx_avarage = (cp->rx_avarage/10)*7 + (cp->rx_byte_cnt/10)*3;
	if (cp->rx_avarage > cp->rx_peak)
		cp->rx_peak = cp->rx_avarage;
	cp->rx_byte_cnt = 0;   
	//eth1
	cp_next->tx_avarage = (cp_next->tx_avarage/10)*7 + (cp_next->tx_byte_cnt/10)*3;
	if (cp_next->tx_avarage > cp_next->tx_peak)
		cp_next->tx_peak = cp_next->tx_avarage;
	cp_next->tx_byte_cnt = 0;
    
	cp_next->rx_avarage = (cp_next->rx_avarage/10)*7 + (cp_next->rx_byte_cnt/10)*3;
	if (cp_next->rx_avarage > cp_next->rx_peak)
		cp_next->rx_peak = cp_next->rx_avarage;
	cp_next->rx_byte_cnt = 0;    
#endif

#ifdef BR_SHORTCUT
	if (pkt_cnt > 100)
		enable_brsc=1;
	else
		enable_brsc=0;
#endif

/*
#ifdef CONFIG_RTK_MESH

	static int link_state = 0;
	int current_ls;

	current_ls = check_LAN_linkstatus_any();
	if (current_ls!=link_state){
		br_signal_pathsel();
		link_state = current_ls;
	}
	
	
#endif
*/
#ifdef __KERNEL__
#ifdef CONFIG_RTL8196C_REVISION_B
	if ((REG32(REVR) == RTL8196C_REVISION_A) && (eee_enabled)) {
		int i, curr_sts;
		uint32 reg;

		/* 
			prev_port_sts[] = 0, current = 0	:	do nothing
			prev_port_sts[] = 0, current = 1	:	update prev_port_sts[]
			prev_port_sts[] = 1, current = 0	:	update prev_port_sts[], disable EEE
			prev_port_sts[] = 1, current = 1	:	enable EEE if EEE is disabled
		 */
		for (i=0; i<MAX_PORT_NUMBER; i++)
		{
			curr_sts = (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp) >> 4;

			if ((prev_port_sts[i] == 1) && (curr_sts == 0)) {
				// disable EEE MAC
				REG32(EEECR) = (REG32(EEECR) & ~(0x1f << (i * 5))) | 
					((EN_P0_FRC_EEE|FRC_P0_EEE_100) << (i * 5));
				//printk("  disable EEE for port %d\n", i);
			}
			else if ((prev_port_sts[i] == 1) && (curr_sts == 1)) {
				reg = REG32(EEECR);
				if ((reg & (1 << (i * 5))) == 0) {
					// enable EEE MAC
					REG32(EEECR) = (reg & ~(0x1f << (i * 5))) | 
						((FRC_P0_EEE_100|EN_P0_TX_EEE|EN_P0_RX_EEE) << (i * 5));
					//printk("  enable EEE for port %d\n", i);
				}
			}			
			prev_port_sts[i] = curr_sts;
		}
		//printk(" %d %d %d %d %d\n", port_sts[0], port_sts[1], port_sts[2], port_sts[3], port_sts[4]);
	}
#endif
#endif

	#if defined(CONFIG_RTL_8367R_SUPPORT)&&defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
	rtk_refinePortDuplexMode();
	#endif
	
	#ifdef CONFIG_RTL_8196E
	refine_phy_setting();
	#endif

    #if defined(CONFIG_RTL_8881A_CUTE_MAHJONG_ESD)
    rtl_CuteMahjongEsdTimerHandle();
    #endif

#ifdef CONFIG_RTL_REPORT_LINK_STATUS
	if(block_link_change > 0)
		block_link_change = 0;

	#if defined(CONFIG_RTL_8367R_SUPPORT)
	rtl_check_8367r_link_status();
	#endif
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#ifndef CONFIG_RTL_8367R_SUPPORT
	rtl819xD_checkPhyCbSnr();
#endif
#endif 

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT) && defined(CONFIG_RTL819X_GPIO)
	if (gpio_simulate_mdc_mdio)
		ckeck_link_speed();
#endif
	
#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG

    #ifdef CONFIG_RTL_8196C_ESD_NEW
    	if ( (REG32(PCRP4) & EnablePHYIf) == 0 ) 
    #else
    	if (check_tx_desc_hang()) 
    #endif		
    	{	

    #ifdef CONFIG_RTL_8196C_ESD_NEW
            #define _8196C_SYS_SW_RESET	BIT(9)
            #define SYS_CLK_MAG		0xb8000010
            #define PIN_MUX_SEL		0xb8000040
            #define PORT_PABCD_BASE		10	// Base of P0~P1 at PABCD

    		uint32 val;

    		// set led off
    		val = REG32(PIN_MUX_SEL); 
    		REG32(PIN_MUX_SEL) = val | 0x3ff;
    		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~((0x1f)<<PORT_PABCD_BASE)));	//set GPIO pin
    		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x1f)<<PORT_PABCD_BASE));//output pin
    		WRITE_MEM32(PABCD_DAT, (READ_MEM32(PABCD_DAT) | ((0x1f)<<PORT_PABCD_BASE)));//set 1: off
    		
    	  	REG32(SYS_CLK_MAG) &= (~(_8196C_SYS_SW_RESET));
    		mdelay(3);
    		REG32(SYS_CLK_MAG) |= (_8196C_SYS_SW_RESET);
    #endif	
    		hangup_reinit = 1;
    #ifndef __ECOS
    		skip_free_skb_check = 1;
    #endif
    		reinit_eth_intf();
    		hangup_reinit = 0;
    #ifndef __ECOS
    		skip_free_skb_check = 0;
    #endif

    #ifdef CONFIG_RTL_8196C_ESD_NEW
    		REG32(PIN_MUX_SEL) = val;
    #endif

    	}
#endif //end of CONFIG_RTK_CHECK_ETH_TX_HANG

	mod_timer(&cp->expire_timer, jiffies + 100);	
#else //else of ETH_NEW_FC
	mod_timer(&cp->expire_timer, jiffies + 100);

	#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG
	if (check_tx_desc_hang()) {
		reinit_eth_intf();
	}
	#endif
#endif//end of ETH_NEW_FC

	//spin_unlock_irqrestore(&cp->lock, flags);   
	restore_flags(flags);					
}
#endif 

//---------------------------------------------------------------------------

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
extern void set_phy_pwr_save(int id, int val);
static void power_save_timer(unsigned long task_priv)
{
	uint32 flags;
    	struct re_private *cp = ((struct net_device *)task_priv)->priv;

	save_and_cli(flags);

	set_phy_pwr_save(port_pwr_save_low, 1); 

	port_pwr_save_low = (port_pwr_save_low + 1) % 5;
	set_phy_pwr_save(port_pwr_save_low, 0); 

	mod_timer(&cp->expire_timer2, jiffies + (total_time_for_5_port / 5 / 10));

	restore_flags(flags);					
}
#endif

//---------------------------------------------------------------------------
#ifdef CONFIG_RTL_KERNEL_MIPS16_DRVETH
__NOMIPS16
#endif 
#ifdef __KERNEL__
static 
#endif
int rtl865x_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;
#ifndef CONFIG_RTL865X_HW_TABLES
	rtl_netif_param_t np;
	int ret;
#endif

#ifdef __ECOS
	p = (unsigned char *)addr;
#else
	p = (unsigned char *)(((struct sockaddr *)addr)->sa_data);
#endif

	//save_flags(flags); cli();
	save_and_cli(flags);
	for (i = 0; i<6; ++i) {
		dev->dev_addr[i] = p[i];
	}

	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	i = dev->name[3] - '0';
	if ((i >=0) && (i <5))
		memcpy(vlanSetting[i].mac, dev->dev_addr, 6);

	if (i == 0) {
		memcpy(vlanSetting[2].mac, dev->dev_addr, 6);
		memcpy(vlanSetting[3].mac, dev->dev_addr, 6);
		memcpy(vlanSetting[4].mac, dev->dev_addr, 6);
	}

	#else
	if (!strcmp(dev->name, "eth0"))
		i = 0;
	else
		i = 1;	
	memcpy(vlanSetting[i].mac, dev->dev_addr, 6);
	#endif

#ifdef CONFIG_RTL865X_HW_TABLES /* Forrest */
	if (!strcmp(dev->name, "eth0"))
		rtl865x_updateNetIfTabMac("br0", p); /* Alias eth0 to br0 */
	else
		rtl865x_updateNetIfTabMac(dev->name, p);
#elif defined(CONFIG_RTL_LAYERED_DRIVER) 
	{
		rtl865x_netif_t netif;
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		memcpy(netif.macAddr.octet, dev->dev_addr, ETHER_ADDR_LEN);
		memcpy(netif.name, dev->name, MAX_IFNAMESIZE);
		rtl865x_setNetifMac(&netif);
	}
#else
	swCore_init(-1); // reset interface table
	for (i=0; i<totalVlans; i++) {    
		bzero((void *) &np, sizeof(rtl_netif_param_t));
		np.vid = vlanSetting[i].vid;
		np.valid = 1;
		#if defined(CONFIG_HW_MULTICAST_TBL) || defined(CONFIG_RTL8198)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
		np.enableRoute = 1;
		#else
		np.enableRoute = 0;
		#endif

		#if defined(CONFIG_RTL_VLAN_SUPPORT)
		if (i == 1) { // WAN (eth1)
	   		np.inAclEnd = 1;
	   		np.inAclStart = 1;
		}else{ // LAN (eth0)
	   		np.inAclStart = 10;
	   		np.inAclEnd = 10;
		}
		#else
		{
			if (i == 1) { // WAN (eth1)
			   np.inAclEnd = 1;
			   np.inAclStart = 0;
			}else{ // LAN (eth0)
			   np.inAclStart = 10;
			   np.inAclEnd = 12;
			}
		}
		#endif
		
		np.outAclEnd = 0;
		np.outAclStart = 0;
		memcpy(&np.gMac, &vlanSetting[i].mac[0], 6);                        
		np.macAddrNumber = 1;
		np.mtu = 1500;
		ret = swCore_netifCreate(i, &np);
		if ( ret != 0 ) {
			printk("865x-nic: swCore_netifCreate() failed:%d\n", ret );
			return -1;
		}
	}
#endif /* Forrest */
	restore_flags(flags);
	return 0;
}


//---------------------------------------------------------------------------
uint16 _fid = 1;

#ifdef CONFIG_RTL865X_HW_TABLES

#define INIT_CHECK(expr) do {\
    if(((int32)expr)!=SUCCESS){\
        rtlglue_printf("Error >>> %s:%d failed !\n", __FUNCTION__,__LINE__);\
            return FAILED;\
    }\
}while(0)

int re_init_vlanTable(int mode)
{
	#ifdef CONFIG_RTL8196B
	#ifdef CONFIG_RTL865X_WTDOG
	unsigned long wtval;
	#endif
	#endif
	
	if (mode == 0) { /* Gateway mode */
		lrConfig[0].memPort = ALL_PORTS & (~BIT(wan_port));
		lrConfig[0].untagSet = lrConfig[0].memPort;
		lrConfig[1].memPort = BIT(wan_port);
		lrConfig[1].untagSet = lrConfig[1].memPort;
		#ifndef CONFIG_RTL865X_HW_PPTPL2TP
		lrConfig[2].memPort = BIT(wan_port);
		lrConfig[2].untagSet = lrConfig[2].memPort;
		#endif
  
		lrConfig[0].fid = 1;
		_fid = 1;
		INIT_CHECK(rtl865x_lightRomeConfig());
		
	}
	else {
		#ifdef TWO_VLANS_IN_BRIDGE_MODE
			lrConfig[0].memPort = ALL_PORTS & (~BIT(wan_port));
			lrConfig[0].untagSet = lrConfig[0].memPort;
			lrConfig[1].memPort = BIT(wan_port) | CPU_PORT;
			lrConfig[1].untagSet = lrConfig[1].memPort;
			#ifndef CONFIG_RTL865X_HW_PPTPL2TP
			lrConfig[2].memPort = BIT(wan_port) | CPU_PORT;
			lrConfig[2].untagSet = lrConfig[2].memPort;
	  		#endif
			lrConfig[0].fid = 0;
			_fid = 0;
			INIT_CHECK(rtl865x_lightRomeConfig());

		#else

			lrConfig[0].memPort = ALL_PORTS;
			lrConfig[0].untagSet = lrConfig[0].memPort;
			lrConfig[1].memPort = 0;
			lrConfig[1].untagSet = 0;
			#ifndef CONFIG_RTL865X_HW_PPTPL2TP
			lrConfig[2].memPort = 0;
			lrConfig[2].untagSet = 0;
			#endif
	            
			lrConfig[0].fid = 0;
			_fid = 0;
			#ifdef CONFIG_RTL8196B

			#ifdef CONFIG_RTL865X_WTDOG
			wtval = *((volatile unsigned long *)WDTCNR);
			*((volatile unsigned long *)WDTCNR) = (WDSTOP_PATTERN << WDTE_OFFSET);
			#endif
			
			FullAndSemiReset();
			rtl8651_extDeviceinit();
			rtl865x_lightRomeInit(); 
			rtl865x_lightRomeConfig();
			//rtl8651_extDeviceInitTimer(); 
			#ifdef CONFIG_RTL865X_WTDOG
			*((volatile unsigned long *)WDTCNR) |=  1 << 23;
			*((volatile unsigned long *)WDTCNR) = wtval;
			#endif
			
			#else
			INIT_CHECK(rtl865x_lightRomeConfig());
			
			#endif
		#endif
		
	}

	return 0;	
}

#endif

static int rtl865x_init_hw(void)
{
#ifndef CONFIG_RTL865X_HW_TABLES
	rtl_netif_param_t np;
	rtl_vlan_param_t vp;
	struct  init_vlan_setting *setting;    
	unsigned int ret;    
	int ethno;
#endif
	
	if (swCore_init(totalVlans)) {
		printk("865x-nic: swCore_init() failed!\n");   
		return -1;
	}

	/* Initialize NIC module */
	if (RTL_swNic_init(rxRingSize, NUM_RX_PKTHDR_DESC, txRingSize, MBUF_LEN))  {
		printk("865x-nic: RTL_swNic_init failed!\n");            
		return -1;
	}

#ifndef CONFIG_RTL865X_HW_TABLES /* Forrest */
	setting = vlanSetting;
	for (ethno = 0; ethno < totalVlans; ethno++) {        
		/* Create NetIF */
		bzero((void *) &np, sizeof(rtl_netif_param_t));
		np.vid = setting[ethno].vid;
		np.valid = 1;
		#if defined(CONFIG_HW_MULTICAST_TBL) || defined(CONFIG_RTL8198)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
		np.enableRoute = 1;
		#else
		np.enableRoute = 0;
		#endif

		#if defined(CONFIG_RTL_VLAN_SUPPORT)
		if (ethno == 1) { // WAN (eth1)
	   		np.inAclEnd = 1;
	   		np.inAclStart = 1;
		}else{ // LAN (eth0)
	   		np.inAclStart = 10;
	   		np.inAclEnd = 10;
		}
		#else
		if (ethno == 1) { // WAN (eth1)
		   np.inAclEnd = 1;
		   np.inAclStart = 0;
		}else{ // LAN (eth0)
		   np.inAclStart = 10;
		   np.inAclEnd = 12;
		}
		#endif
		np.outAclEnd = 0;
		np.outAclStart = 0;

		memcpy(&np.gMac, &setting[ethno].mac[0], 6);
                        
		np.macAddrNumber = 1;
		np.mtu = 1500;
		ret = swCore_netifCreate(ethno, &np);
		if ( ret != 0 ) {
			printk("865x-nic: swCore_netifCreate() failed:%d\n", ret );
			return -1;
		}
            
		/* Create VLAN */
		bzero((void *) &vp, sizeof(rtl_vlan_param_t));
		#ifdef CONFIG_RTL8196_RTL8366
		vp.egressUntag &= ~(0x41);
		vp.memberPort = 0x41;
		vp.fid = 0;
		#else
		vp.egressUntag = setting[ethno].portmask;
		vp.memberPort = setting[ethno].portmask;
		vp.fid = ethno;
		#endif

		ret = swCore_vlanCreate(setting[ethno].vid, &vp);  //P1-P4

		if ( ret != 0 ) {           
			printk("865x-nic: swCore_vlanCreate() failed:%d\n", ret );
			return -1;  
		}
	}				
#endif /* Forrest */

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if ((eth_flag & BIT(3)) && (savOP_MODE_value==2))
		REG32(FFCR) = REG32(FFCR)  | IPMltCstCtrl_Enable;
	else
		REG32(FFCR) = REG32(FFCR)  & ~IPMltCstCtrl_Enable;		
			
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (L2_table_disabled) 
		REG32(SWTCR0) = REG32(SWTCR0)  | EnUkVIDtoCPU;
	else
		REG32(SWTCR0) = REG32(SWTCR0)  & (~EnUkVIDtoCPU);
#endif

#ifdef CONFIG_RTL_VLAN_SUPPORT
	if (rtl_vlan_support_enable) 
		REG32(SWTCR0) = REG32(SWTCR0)  | EnUkVIDtoCPU;
	else
		REG32(SWTCR0) = REG32(SWTCR0)  & (~EnUkVIDtoCPU);
#endif

#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
	{
	int port;
	
	#if defined(CONFIG_RTL_8881A)
	REG32(PIN_MUX_SEL2) = (REG32(PIN_MUX_SEL2) & ~(0x3FFF)) | (0x36DB);
	#endif
	
	for (port=0; port<RTL8651_PHY_NUMBER; port++)
		init_led_ctrl(port);
	}
#endif
	return 0;	
}


//---------------------------------------------------------------------------
#ifdef CONFIG_RTL865X_HW_TABLES		
static void reset_hw_mib_counter(struct net_device *dev)
{
	int vid, port;
	
	for (vid=0; vid<2; vid++) {
		if (!memcmp(vlanSetting[vid].mac, dev->dev_addr, 6))
			break;
	}
	if (vid == 2) {
		printk("%s: get vid error!\n", __FUNCTION__);
		return;
	}
	for (port=0; port<MAX_PORT_NUMBER; port++) {
		if 	(vlanSetting[vid].portmask & (1<<port)) 
		   WRITE_MEM32(MIB_CONTROL, (1<<port*2) | (1<<(port*2+1)));		
	}
}
#endif

#ifdef CONFIG_RTL_HARDWARE_NAT
static void reset_hw_mib_counter(struct net_device *dev)
{
	int port;
	struct re_private *cp;

	cp = (struct re_private *)dev->priv;
	for (port=0; port<RTL8651_PORT_NUMBER+3; port++)
	{
		if (cp->port & (1<<port)){
			#ifdef CONFIG_RTL_8367R_SUPPORT
			{
				extern int rtk_stat_port_reset(int port);
				rtk_stat_port_reset(port);
			}
			#endif					
		       WRITE_MEM32(MIB_CONTROL, (1<<port*2) | (1<<(port*2+1)));
		}
		
		return;
	}
}
#endif

//---------------------------------------------------------------------------
static int rtl865x_open(struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	uint32 flags;
	int rc;

	if (cp->opened)
		return 0;

	save_and_cli(flags);

	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT) || defined(CONFIG_RTL_VLAN_SUPPORT)
	if (dev->name[3] < '2') 
	#endif
	
//    if (IS_FIRST_DEV(cp)) { 
	if (!NEXT_DEV(cp) || !NEXT_CP(cp)->opened) {     // this is the first open dev
#ifdef RTK_QUE
		rtk_queue_init(&rx_skb_queue);  
#else
		skb_queue_head_init(&rx_skb_queue);
#endif

		//spin_lock_irqsave (&cp->lock, flags);

		rc = rtl865x_init_hw();

		//spin_unlock_irqrestore(&cp->lock, flags);		

		if (rc) {
			printk("rtl865x_init_hw() failed!\n");
			restore_flags(flags);
			return 1;
		}

#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG	
		if (!hangup_reinit)
#endif		
#endif
		{
#ifdef __KERNEL__
#ifdef RX_TASKLET
			tasklet_init(&cp->rx_dsr_tasklet, interrupt_dsr_rx, (unsigned long)cp);
#endif

#ifdef TX_TASKLET
			tasklet_init(&cp->tx_dsr_tasklet, interrupt_dsr_tx, (unsigned long)cp);
#endif
#endif
		}

#ifdef __KERNEL__
		rc = request_irq(dev->irq, interrupt_isr, SA_INTERRUPT, dev->name, dev);
		if (rc) {
			printk("request_irq() error!\n");
			goto err_out_hw;
		}
		cp->irq_owner =1;
#endif
		swCore_start();			
	}
    
	cp->opened = 1;

#if defined(CONFIG_RTL_HARDWARE_NAT)	
	reset_hw_mib_counter(dev);
#endif

	netif_start_queue(dev);
#ifdef CONFIG_RTK_VOIP_PORT_LINK
	if ( !link_state_timer_work)
	{
		voip_rtnl = netlink_kernel_create(NETLINK_ROUTE, NULL);
		init_timer(&link_state_timer);
		link_state_timer.expires = jiffies + 100;
		link_state_timer.data = dev;
		link_state_timer.function = link_state_timer_action;
		mod_timer(&link_state_timer, jiffies + 100);
		link_state_timer_work = 1;
	}
#endif

#if defined(DYNAMIC_ADJUST_TASKLET) || defined(BR_SHORTCUT)  || defined(CONFIG_RTL8196C_REVISION_B)|| defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)  || defined(CONFIG_RTL_8197F)
#if !defined(CONFIG_RTL8186_TR) && !defined(CONFIG_RTL8196B_TR) && !defined(CONFIG_RTL865X_AC) && !defined(CONFIG_RTL865X_KLD) && !defined(CONFIG_RTL8196C_EC)
	if (dev->name[3] == '0') 
#endif		
	{	

#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG	
		if (!hangup_reinit)
#endif		
#endif
		{
			init_timer(&cp->expire_timer);
#ifdef __KERNEL__
			cp->expire_timer.expires = jiffies + 100;
#endif
			cp->expire_timer.data = (unsigned long)dev;
			cp->expire_timer.function = one_sec_timer;

			mod_timer(&cp->expire_timer, jiffies + 100);
		}

#ifdef DYNAMIC_ADJUST_TASKLET			
		rx_cnt = 0;
#endif
#ifdef BR_SHORTCUT
		pkt_cnt = 0;
#endif
	}
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
	if (REG32(REVR) == RTL8196C_REVISION_B) {
		if (dev->name[3] == '0') 
		{	
			init_timer(&cp->expire_timer2);
#ifdef __KERNEL__		
			cp->expire_timer2.expires = jiffies + 100;
#endif
			cp->expire_timer2.data = (unsigned long)dev;
			cp->expire_timer2.function = power_save_timer;

			mod_timer(&cp->expire_timer2, jiffies + 100);			
		}
	}
#endif

	#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
	/*should add default route to cpu....*/
	if(savOP_MODE_value == 2){	//GATEWAY_MODE
		//rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
	}
	#endif

	restore_flags(flags);
	return 0;

#ifdef __KERNEL__
err_out_hw:
	rtl865x_stop_hw(dev, cp);
	restore_flags(flags);
	return rc;
#endif
}

//---------------------------------------------------------------------------
static int rtl865x_close(struct net_device *dev)
{
	struct re_private *cp = dev->priv;
	uint32 flags;

	if (!cp->opened)
		return 0;

	save_and_cli(flags);

#ifdef CONFIG_RTK_VOIP_PORT_LINK
	if (link_state_timer_work){
		if (timer_pending(&link_state_timer)){
       		    del_timer_sync(&link_state_timer);
       		    link_state_timer_work = 0;
       		}
       	}
#endif
	
	netif_stop_queue(dev);

	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT) || defined(CONFIG_RTL_VLAN_SUPPORT) 
	if (dev->name[3] < '2') 
	#endif

	if (!NEXT_DEV(cp) || !NEXT_CP(cp)->opened) {		
		rtl865x_stop_hw(dev, cp);       
						
		free_irq(dev->irq, GET_IRQ_OWNER(cp));				
#ifdef __KERNEL__
		((struct re_private *)GET_IRQ_OWNER(cp)->priv)->irq_owner = 0;        
#endif

#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG
		if (!hangup_reinit)
#endif		
#endif
		{
#ifdef __KERNEL__
#ifdef RX_TASKLET
			tasklet_kill(&cp->rx_dsr_tasklet);	
#endif

#ifdef TX_TASKLET
			tasklet_kill(&cp->tx_dsr_tasklet);
#endif
#endif
		}

#ifdef ETH_NEW_FC
#ifdef DELAY_REFILL_ETH_RX_BUF
		during_close = 1;
#endif
		free_rx_skb();
		free_pending_tx_skb();		

#ifdef DELAY_REFILL_ETH_RX_BUF
		during_close = 0;
#endif
#else
		free_rx_skb();
#endif
	}

#ifdef __KERNEL__
	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
#endif
	
	cp->opened = 0;
		
#if defined(DYNAMIC_ADJUST_TASKLET) || defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(BR_SHORTCUT) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) ||defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL8196C_KLD)|| defined(CONFIG_RTL8198) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
#ifdef ETH_NEW_FC
#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG	
	if (!hangup_reinit)
#endif		
#endif
	if (timer_pending(&cp->expire_timer))
		del_timer_sync(&cp->expire_timer);
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
	if (REG32(REVR) == RTL8196C_REVISION_B) {
		if (timer_pending(&cp->expire_timer2))
			del_timer_sync(&cp->expire_timer2);	
	}
#endif

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)	
	reset_hw_mib_counter(dev);
#endif

	restore_flags(flags);				

	return 0;
}
#if (defined(CONFIG_RTL_CUSTOM_PASSTHRU))


static int re865x_pseudo_open (struct net_device *dev)
{
	struct re_private *cp;

	cp = dev->priv;
	//cp = netdev_priv(dev);
	if (cp->opened)
		return SUCCESS;

	cp->opened = 1;
	netif_start_queue(dev);
	return SUCCESS;
}


static int re865x_pseudo_close (struct net_device *dev)
{
	struct re_private *cp;

	cp = dev->priv;
//	cp = netdev_priv(dev);

	if (!cp->opened)
		return SUCCESS;
	netif_stop_queue(dev);

	memset(&cp->net_stats, '\0', sizeof(struct net_device_stats));
	cp->opened = 0;

#ifdef BR_SHORTCUT
	if (dev == cached_dev)
		cached_dev=NULL;
#endif
#ifdef BR_SHORTCUT_C2
	if (dev == cached_dev2)
		cached_dev2=NULL;
#endif
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)

static int custom_createPseudoDevForPassThru(void)
{
		struct net_device *dev, *wanDev;
		struct re_private *dp;
		int rc, i;
		unsigned long		flags;

		wanDev = NULL;
		
		/*	find wan device first	*/
		for(i=0;i<ETH_INTF_NUM;i++)
		{
			if (reNet[i]) {
				dp = ((struct re_private *)reNet[i]->priv);
				if (rtl_isWanDev(dp)==TRUE)
				{
					wanDev = reNet[i];
					break;
				}
			}
		}

		/*	can't find any wan device, just return	*/
		if (wanDev==NULL){
			diag_printf("can't find any wan device.[%s]:[%d].\n",__FUNCTION__,__LINE__);
			return -1;
		}	
		
		dev = alloc_etherdev(sizeof(struct re_private));
		if (!dev){
			diag_printf("failed to allocate passthru pseudo dev.\n");
			return -1;
		}
		
		//strcpy(dev->name, "peth%d");
		/*	default set lan side mac		*/
		for (i = 0; i < 6; i++)
			((u8 *)(dev->dev_addr))[i] =vlanSetting[0].mac[i];
		dp = dev->priv;
		memset(dp,0,sizeof(*dp));
		dp->dev = wanDev;

		dev->open = re865x_pseudo_open;
		dev->stop = re865x_pseudo_close;
		//dev->set_multicast_list = NULL;
		dev->hard_start_xmit = rtl865x_start_xmit;
		dev->get_stats = rtl865x_get_stats;
		dev->do_ioctl = re865x_ioctl;
#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
		dev->vlan_rx_register = cp_vlan_rx_register;
		dev->vlan_rx_kill_vid = cp_vlan_rx_kill_vid;
#endif


#ifdef CP_VLAN_TAG_USED
		dev->features |= NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
#endif
#ifdef __ECOS

		dev->isr = interrupt_isr;	
		dev->dsr= interrupt_dsr;
		dev->can_xmit = can_xmit;

		strcpy(dev->name, "peth0");	
#endif	
		pDev = dev;
		diag_printf("[%s] added, mapping to [%s]...\n", dev->name, dp->dev->name);
		return 0;
}




int32 rtl8651_customPassthru_init(void)
{
	#ifdef CONFIG_PROC_FS
	
	oldStatus=0;
	isCreated=0;
	res_custom_passthru = create_proc_entry("custom_Passthru", 0, NULL);	
	if(res_custom_passthru)
	{
		res_custom_passthru->read_proc = custom_Passthru_read_proc;
		res_custom_passthru->write_proc = custom_Passthru_write_proc;
	}

	#endif
	passThru_flag[0]=0;
#if 1	
	rtl8651_defineProtocolBasedVLAN( IP6_PASSTHRU_RULEID, 0x0, 0x86DD );
	
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	rtl8651_defineProtocolBasedVLAN( PPPOE_PASSTHRU_RULEID1, 0x0, __constant_htons(ETH_P_PPP_SES) );
	rtl8651_defineProtocolBasedVLAN( PPPOE_PASSTHRU_RULEID2, 0x0, __constant_htons(ETH_P_PPP_DISC) );
#endif
#endif

	//pDev won't change???
	if(pDev==NULL)
		custom_createPseudoDevForPassThru();
	return 0;
}

static void passthru_vlan_create(void)
{
	rtl_netif_param_t np;
	rtl_vlan_param_t vp;

	/* Create NetIF */
	bzero((void *) &np, sizeof(rtl_netif_param_t));
	np.vid = PASSTHRU_VLAN_ID;
	np.valid = 1;
	np.enableRoute = 1;
	np.inAclEnd = 2;
	np.inAclStart = 1;
	np.outAclEnd = 0;
	np.outAclStart = 0;
	memcpy(&np.gMac, passthru_vlan_mac, 6);
                        
	np.macAddrNumber = 1;
	np.mtu = 1500;
	swCore_netifCreate(RTL865XC_NETINTERFACE_NUMBER - 1, &np);
            
	/* Create VLAN */
	bzero((void *) &vp, sizeof(rtl_vlan_param_t));
	vp.egressUntag = ALL_PORTS;
	vp.memberPort = ALL_PORTS;
	vp.fid = 0;
	swCore_vlanCreate(PASSTHRU_VLAN_ID, &vp);

	return;
}

void rtl8651_customPassthru_infoSetting(void)
{
	int i=0;
	int flag;
	flag= passThru_flag[0] ;
	rtl_vlan_param_t vp;
	
	if(flag ^ old_passThru_flag)
	{	
		#if 1//def CONFIG_HW_PROTOCOL_VLAN_TBL
		//IPv6 PassThru
		if((flag & IP6_PASSTHRU_MASK) ^ (old_passThru_flag & IP6_PASSTHRU_MASK)) 
		{			
			if(flag & IP6_PASSTHRU_MASK)
			{//add				
				for(i=0; i<RTL865XC_PORT_NUMBER; i++)
				{
					rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, TRUE, PASSTHRU_VLAN_ID);
				}	
				
			}
			else
			{//delete
				for(i=0; i<RTL865XC_PORT_NUMBER; i++)
				{
					rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, FALSE, PASSTHRU_VLAN_ID);
				}
			}
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		//PPPoE PassThru
		if((flag & PPPOE_PASSTHRU_MASK) ^ (old_passThru_flag & PPPOE_PASSTHRU_MASK)) 
		{			
			if(flag & PPPOE_PASSTHRU_MASK)
			{//add				
				for(i=0; i<RTL865XC_PORT_NUMBER; i++)
				{
					rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, TRUE, PASSTHRU_VLAN_ID);
					rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, TRUE, PASSTHRU_VLAN_ID);
				}
				
			}
			else
			{//delete
				for(i=0; i<RTL865XC_PORT_NUMBER; i++)
				{
					rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, FALSE, PASSTHRU_VLAN_ID);
					rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, FALSE, PASSTHRU_VLAN_ID);
				}
			}
		}
		#endif
		#endif
		
		
	}
	
	old_passThru_flag=flag;

}

void passthru_show(void)
{
	diag_printf("\npassthru:%d   ",passThru_flag[0]);
	if (passThru_flag[0]&IP6_PASSTHRU_MASK){
		diag_printf("ipv6 passthru support");
		#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if(passThru_flag[0]&PPPOE_PASSTHRU_MASK)
			diag_printf("& PPPOE passthru support");
		#endif
		diag_printf("\n");
	}	
	else
	{
		#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if(passThru_flag[0]&PPPOE_PASSTHRU_MASK)
			diag_printf("PPPOE passthru support\n");
		#endif
	}
}	
static void __exit rtl8651_customPassthru_exit(void)
{	
	#ifdef CONFIG_PROC_FS

	if (res_custom_passthru) {
		remove_proc_entry("custom_Passthru", res);
		res_custom_passthru = NULL;
	}
	#endif
}
#endif

//---------------------------------------------------------------------------
#ifdef CONFIG_PROC_FS
#ifdef CONFIG_RTL_KERNEL_MIPS16_DRVETH
__NOMIPS16
#endif 
static int write_proc(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    unsigned char tmpbuf[100];

    if (count < 2)
        return -EFAULT;

    if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
        sscanf(tmpbuf, "%d", &eth_flag);

		if ((eth_flag & TASKLET_MASK) == 1)
			rx_tasklet_enabled = 0;
		if ((eth_flag & TASKLET_MASK) == 2)
			rx_tasklet_enabled = 1;	

#if defined(CONFIG_RTL865X_HW_TABLES) || defined(CONFIG_HW_MULTICAST_TBL)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if (eth_flag & BIT(3))
			REG32(FFCR) = REG32(FFCR)  | IPMltCstCtrl_Enable;
		else
			REG32(FFCR) = REG32(FFCR)  & ~IPMltCstCtrl_Enable;		
#endif
				
        return count;
    }
    return -EFAULT;
}

//---------------------------------------------------------------------------
#ifdef DYNAMIC_ADJUST_TASKLET
static int write_proc_rxthres(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    unsigned char tmpbuf[100];

    if (count < 2)
        return -EFAULT;
    if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
        sscanf(tmpbuf, "%d", &rx_pkt_thres);
        if (rx_pkt_thres) {
				rx_cnt = 0;
				eth_flag = 0 | (eth_flag & ~TASKLET_MASK);
        }       
        return count;
    }
    return -EFAULT;
}
#endif

//---------------------------------------------------------------------------
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
static int read_proc_eth_stats(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{
	int len;

	#ifdef CONFIG_RTK_CHECK_ETH_TX_HANG
	len = sprintf(page, "  eth_skb_free_num: %d, reused_cnt: %d, re_init_cnt: %d\n", eth_skb_free_num+rx_skb_queue.qlen, reused_skb_num, tx_hang_cnt);  
	#else
	len = sprintf(page, "  eth_skb_free_num: %d, reused_cnt: %d\n", eth_skb_free_num+rx_skb_queue.qlen, reused_skb_num);  
	#endif
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}
#endif

//---------------------------------------------------------------------------
#ifdef CONFIG_PROC_FS
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
static int read_proc_stats(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;
    int len;

    len = sprintf(page, "%d %d %d %d\n", cp->tx_avarage, cp->tx_peak, cp->rx_avarage, cp->rx_peak);  
    if (len <= off+count) 
        *eof = 1;      
    *start = page + off;      
    len -= off;      
    if (len > count) 
        len = count;      
    if (len < 0) 
        len = 0;      
    return len; 

}

//---------------------------------------------------------------------------
static int write_proc_stats(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;

    cp->tx_avarage = cp->tx_peak = cp->tx_byte_cnt = 0;
    cp->rx_avarage = cp->rx_peak = cp->rx_byte_cnt = 0;

    return count;
}
#endif // CONFIG_RTL8186_TR

#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static int read_proc_stats(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;
    int len;

    len = sprintf(page, "%d %d %d %d %ld\n", cp->tx_avarage, cp->tx_peak, cp->rx_avarage, cp->rx_peak, cp->net_stats.rx_packets);  
    if (len <= off+count) 
        *eof = 1;      
    *start = page + off;      
    len -= off;      
    if (len > count) 
        len = count;      
    if (len < 0) 
        len = 0;      
    return len; 

}

//---------------------------------------------------------------------------
static int write_proc_stats(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;

    cp->tx_avarage = cp->tx_peak = cp->tx_byte_cnt = 0;
    cp->rx_avarage = cp->rx_peak = cp->rx_byte_cnt = 0;
#if defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	memset(&cp->net_stats, 0, sizeof(struct net_device_stats));
#endif

    return count;
}
#endif // CONFIG_RTL865X_AC || CONFIG_RTL865X_KLD


#ifdef CONFIG_RTK_VLAN_SUPPORT
static int read_proc_vlan(char *page, char **start, off_t off,
        int count, int *eof, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;
    int len;

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
	len = sprintf(page, "gvlan=%d, lan=%d, vlan=%d, tag=%d, vid=%d, priority=%d, cfi=%d, forwarding=%d\n",
#else
	len = sprintf(page, "gvlan=%d, lan=%d, vlan=%d, tag=%d, vid=%d, priority=%d, cfi=%d\n",
#endif
		cp->vlan_setting.global_vlan, cp->vlan_setting.is_lan, cp->vlan_setting.vlan, cp->vlan_setting.tag, 
		cp->vlan_setting.id, cp->vlan_setting.pri, cp->vlan_setting.cfi
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		,cp->vlan_setting.forwarding
#endif
		);
			
    if (len <= off+count) 
        *eof = 1;      
    *start = page + off;      
    len -= off;      
    if (len > count) 
        len = count;      
    if (len < 0) 
        len = 0;      
    return len; 
}


static int write_proc_vlan(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
    struct net_device *dev = (struct net_device *)data;
    struct re_private *cp = dev->priv;
	char tmp[128];

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 128)) {
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		int num = sscanf(tmp, "%d %d %d %d %d %d %d %d",
#else
		int num = sscanf(tmp, "%d %d %d %d %d %d %d",
#endif
			&cp->vlan_setting.global_vlan, &cp->vlan_setting.is_lan, 
			&cp->vlan_setting.vlan, &cp->vlan_setting.tag, 
			&cp->vlan_setting.id, &cp->vlan_setting.pri, 
			&cp->vlan_setting.cfi
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
			, &cp->vlan_setting.forwarding
#endif
			);

#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		if (num !=  8) {
#else
		if (num !=  7) {
#endif
			panic_printk("invalid vlan parameter!\n");
		}
#if defined(CONFIG_RTK_VLAN_ROUTETYPE)
		add_vlan_info(&cp->vlan_setting,dev);
#endif
	}
	return count;	
}
#endif // CONFIG_RTK_VLAN_SUPPORT


static struct proc_dir_entry *proc_wan_port=NULL;

static int read_proc_wan_port(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", wan_port);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

/* 
 * Usage: echo "$WAN_PORT$OP_MODE" > /proc/wan_port, or echo "$WAN_PORT" > /proc/wan_port
 */
#ifdef CONFIG_RTL_KERNEL_MIPS16_DRVETH
__NOMIPS16
#endif 
static int write_proc_wan_port(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[4];
	#ifdef CONFIG_RTL865X_HW_TABLES
    	struct re_private *cp;
	#endif
	int op_mode = 0; //gateway

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 2)) {

		if ((tmp[0] >= '0') && (tmp[0] <= '5')) {
			wan_port = tmp[0] - '0';

			if ((tmp[1] == '1') || (tmp[1] == '2'))
				op_mode = 1; //bridge/WISP

			#ifdef CONFIG_RTL865X_HW_TABLES

			if (op_mode == 0) {
				/* set LAN/WAN port mask */
				vlanSetting[0].portmask = ALL_PORTS & (~BIT(wan_port));
				vlanSetting[1].portmask = BIT(wan_port);
			}
			else {
				vlanSetting[0].portmask = ALL_PORTS;
				vlanSetting[1].portmask = 0;

			}
			
			// eth0(LAN) VLAN
			cp = reNet[0]->priv;
			cp->port = vlanSetting[0].portmask;
		
			// eth1(WAN) VLAN
			cp = reNet[1]->priv;
			cp->port = vlanSetting[1].portmask;

			re_init_vlanTable(op_mode);

			#else
			if (op_mode == 0) 
				op_mode = 2;
			SoftNAT_OP_Mode(op_mode);
			#endif
		}
				
		return count;
	}
	return -EFAULT;
}

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)  ||defined(CONFIG_RTL_8197F)
static struct proc_dir_entry *proc_eth_drv=NULL;
static struct proc_dir_entry *proc_link_speed=NULL;

#ifdef CONFIG_819X_PHY_RW

static struct proc_dir_entry *rtl_phy=NULL;
static struct proc_dir_entry *port_mibStats_root=NULL;
static struct proc_dir_entry *port_mibStats_entry=NULL;

static int32 rtl_phy_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	int		port;
	
	len = sprintf(page, "Port Status:\n");

	for(port=PHY0;port<CPU;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));

		len += sprintf(page+len, "Port%d ", port);
		data0 = regData & PortStatusLinkUp;
		
		if (data0)
			len += sprintf(page+len, "LinkUp | ");
		else
		{
			len += sprintf(page+len, "LinkDown\n");
			continue;
		}		
		//data0 = regData & PortStatusTXPAUSE;  //mark_fix
		data0 = regData & PortStatusDuplex;  		
		len += sprintf(page+len, "	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed %s | ", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));

		regData = READ_MEM32(PCRP0+((port)<<2));
		data0 = regData & EnablePHYIf;
		len += sprintf(page+len, "	EnablePHYIf %s \n", data0?"Enabled":"Disabled");
	}


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 rtl_phy_status_write( struct file *filp, const char *buff,unsigned long len, void *data )
{


	char 		tmpbuf[64];
	uint32	port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int		type;
	int 		port;
	int forceMode = 0;
	int forceLink = 0;
	int forceLinkSpeed = 0;
	int forceDuplex = 0;
	uint32 advCapability = 0;
	int forwardEnable = TRUE;

	#define SPEED10M 	0
	#define SPEED100M 	1
	#define SPEED1000M 	2

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "port", 4))
		{
			port_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			
			if(strcmp(tokptr,"10_half") == 0)
				type = HALF_DUPLEX_10M;
			else if(strcmp(tokptr,"100_half") == 0)
				type = HALF_DUPLEX_100M;
			else if(strcmp(tokptr,"10_full") == 0)
				type = DUPLEX_10M;
			else if(strcmp(tokptr,"100_full") == 0)
				type = DUPLEX_100M;
			else
				type = PORT_AUTO;

			tokptr =  strsep(&strptr," ");
			if(tokptr == NULL)
				goto errout;

			forwardEnable = simple_strtol(tokptr,NULL,0);

			switch(type)
			{
				case HALF_DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_10M);
					break;
				}
				case HALF_DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_100M);
					break;
				}				
				case DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_10M);
					break;
				}
				case DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_100M);
					break;
				}				
				default:	
				{
					forceMode=FALSE;
					forceLink=TRUE;
					/*all capality*/
					advCapability=(1<<PORT_AUTO);		
				}
			}
				
			
			for(port = 0; port < CPU; port++)
			{
				if((1<<port) & port_mask)
				{
					rtl865xC_setAsicEthernetForceModeRegs(port, forceMode, forceLink, forceLinkSpeed, forceDuplex);

					/*Set PHY Register*/
					rtl8651_setAsicEthernetPHYSpeed(port,forceLinkSpeed);
					
					rtl8651_setAsicEthernetPHYDuplex(port,forceDuplex);
					rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
					rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);				
					rtl8651_restartAsicEthernetPHYNway_port(port);					
					rtl865x_setPortForward(port,forwardEnable);
		
				}
			}
			
		}		
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		panic_printk("port status only support \"port\" as the first parameter\n");
		panic_printk("format:	\"port port_mask 10_half/100_half/10_full/100_full/1000_full/auto phyFwd\"\n");
	}
	return len;

}

//mib counter
 int rtl819x_get_port_mibStats(int port , struct port_mibStatistics *port_stats)
 
  {
	  uint32 addrOffset_fromP0 =0;
	  
	  if((port>CPU)||(port_stats==NULL) )
	  {
		  return FAILED;
	  }
	  
	  addrOffset_fromP0= port* MIB_ADDROFFSETBYPORT;
	  
	  memset(port_stats,0,sizeof(struct port_mibStatistics));
	 /* update the mib64 counters from 32bit counters*/
	  //rtl8651_updateAdvancedMibCounter((unsigned long)(&rtl865x_updateMib64Param)); //mark_rm
  
	 port_stats->ifInOctets=rtl8651_returnAsicCounter(OFFSET_IFINOCTETS_P0 + addrOffset_fromP0);
	 //rtl865xC_getAsicCounter64(OFFSET_IFINOCTETS_P0 + addrOffset_fromP0, &(port_stats->ifHCInOctets)); //mark_rm
	 port_stats->ifHCInOctets = port_stats->ifInOctets;
	 
	 port_stats->ifInUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0) ; 
	 //port_stats->ifHCInUcastPkts = rtl865x_updateMib64Param[port].ifHCInUcastPkts;
	 port_stats->ifHCInUcastPkts = port_stats->ifInUcastPkts;
	 
	 port_stats->ifInDiscards=rtl8651_returnAsicCounter(OFFSET_DOT1DTPPORTINDISCARDS_P0+ addrOffset_fromP0);//??
	 port_stats->ifInErrors=(rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) +
						  rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
	 port_stats->ifInMulticastPkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) ;
	 //port_stats->ifHCInMulticastPkts= rtl865x_updateMib64Param[port].ifHCInMulticastPkts;
	 port_stats->ifHCInMulticastPkts = port_stats->ifInMulticastPkts;

	 port_stats->ifInBroadcastPkts= rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ) ;
	 //port_stats->ifHCInBroadcastPkts= rtl865x_updateMib64Param[port].ifHCInBroadcastPkts;
  	  port_stats->ifHCInBroadcastPkts = port_stats->ifInBroadcastPkts;
	 
	 //rtl865xC_getAsicCounter64(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0, &port_stats->etherStatsOctets );
	 port_stats->etherStatsOctets = rtl8651_returnAsicCounter(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0); //replace above , mark_rm
	 //port_stats->etherStatsOctets=rtl865xC_returnAsicCounter64(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0); //not by mark_rm	 
	 port_stats->etherStatsUndersizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsFraments=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts64Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts65to127Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts128to255Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts256to511Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts512to1023Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsPkts1024to1518Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsOversizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsJabbers=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0);
	 port_stats->etherStatusDropEvents=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0);
	 port_stats->dot3FCSErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsSymbolErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0);
	 port_stats->dot3ControlInUnknownOpcodes=rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0);
	 port_stats->dot3InPauseFrames=rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0);
 
	 port_stats->ifOutOctets=rtl8651_returnAsicCounter(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);
	 //rtl865xC_getAsicCounter64(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0, &port_stats->ifHCOutOctets);
	  port_stats->ifHCOutOctets = port_stats->ifOutOctets;
	 
	 // port_stats->ifHCOutOctets=rtl865xC_returnAsicCounter64(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);//not by mark_rm	
	 port_stats->ifOutUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0);
	 //port_stats->ifHCOutUcastPkts = rtl865x_updateMib64Param[port].ifHCOutUcastPkts;
	 port_stats->ifHCOutUcastPkts = port_stats->ifOutUcastPkts;
	 port_stats->ifOutDiscards=rtl8651_returnAsicCounter(OFFSET_IFOUTDISCARDS+ addrOffset_fromP0);
	 port_stats->ifOutErrors=(rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ) +
						  rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
	 port_stats->ifOutMulticastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0);
	 port_stats->ifOutBroadcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0);
	 //port_stats->ifHCOutMulticastPkts=rtl865x_updateMib64Param[port].ifHCOutMulticastPkts;
	 port_stats->ifHCOutMulticastPkts = port_stats->ifOutMulticastPkts;
	 //port_stats->ifHCOutBroadcastPkts=rtl865x_updateMib64Param[port].ifHCOutBroadcastPkts;
	 port_stats->ifHCOutBroadcastPkts = port_stats->ifOutBroadcastPkts;
	 
	 port_stats->ifOutDiscards=rtl8651_returnAsicCounter(OFFSET_IFOUTDISCARDS + addrOffset_fromP0);
	 port_stats->dot3StatsSingleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0+ addrOffset_fromP0);
	 port_stats->dot3StatsMultipleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsDefferedTransmissions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsLateCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3StatsExcessiveCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSEXCESSIVECOLLISIONS_P0 + addrOffset_fromP0);
	 port_stats->dot3OutPauseFrames=rtl8651_returnAsicCounter(OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0);
	 port_stats->dot1dBasePortDelayExceededDiscards=rtl8651_returnAsicCounter(OFFSET_DOT1DBASEPORTDELAYEXCEEDEDDISCARDS_P0 + addrOffset_fromP0);
	 port_stats->etherStatsCollisions=rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0);
 
 
	 //port_stats->ifInUnknownProtos = ethPortInUnknownProtos[port]; //mark_rm
	 port_stats->ifInUnknownProtos = 0;
	 port_stats->dot1dTpLearnedEntryDiscards=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);
	 port_stats->etherStatsCpuEventPkts=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);
 
	  return SUCCESS;
  }

static int show_port_mibStats(struct port_mibStatistics *port_stats, int port, char *page, int off)
{
	int len;
	struct lan_port_status tmp_port_status;
	len = off;
	/*here is in counters  definition*/
	len += sprintf(page+len, "%s%u\n", "ifInOctets=", port_stats->ifInOctets);
	len += sprintf(page+len, "%s%llu\n", "ifHCInOctets=", port_stats->ifHCInOctets);
	len += sprintf(page+len, "%s%u\n", "ifInUcastPkts=", port_stats->ifInUcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInUcastPkts=", port_stats->ifHCInUcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInMulticastPkts=", port_stats->ifInMulticastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInMulticastPkts=", port_stats->ifHCInMulticastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInBroadcastPkts=", port_stats->ifInBroadcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCInBroadcastPkts=", port_stats->ifHCInBroadcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifInDiscards=", port_stats->ifInDiscards);
	len += sprintf(page+len, "%s%u\n", "ifInErrors=", port_stats->ifInErrors);
	len += sprintf(page+len, "%s%llu\n", "etherStatsOctets=", port_stats->etherStatsOctets);
	len += sprintf(page+len, "%s%u\n", "etherStatsUndersizePkts=", port_stats->etherStatsUndersizePkts);
	len += sprintf(page+len, "%s%u\n", "etherStatsFraments=", port_stats->etherStatsFraments);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts64Octets=", port_stats->etherStatsPkts64Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts65to127Octets=", port_stats->etherStatsPkts65to127Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts128to255Octets=", port_stats->etherStatsPkts128to255Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts256to511Octets=", port_stats->etherStatsPkts256to511Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts512to1023Octets=", port_stats->etherStatsPkts512to1023Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsPkts1024to1518Octets=", port_stats->etherStatsPkts1024to1518Octets);
	len += sprintf(page+len, "%s%u\n", "etherStatsOversizePkts=", port_stats->etherStatsOversizePkts);
	len += sprintf(page+len, "%s%u\n", "etherStatsJabbers=", port_stats->etherStatsJabbers);
	len += sprintf(page+len, "%s%u\n", "dot1dTpPortInDiscards=", port_stats->dot1dTpPortInDiscards);
	len += sprintf(page+len, "%s%u\n", "etherStatusDropEvents=", port_stats->etherStatusDropEvents);
	len += sprintf(page+len, "%s%u\n", "dot3FCSErrors=", port_stats->dot3FCSErrors);
	len += sprintf(page+len, "%s%u\n", "dot3StatsSymbolErrors=", port_stats->dot3StatsSymbolErrors);
	len += sprintf(page+len, "%s%u\n", "dot3ControlInUnknownOpcodes=", port_stats->dot3ControlInUnknownOpcodes);
	len += sprintf(page+len, "%s%u\n", "dot3InPauseFrames=", port_stats->dot3InPauseFrames);
	/*here is out counters  definition*/
	len += sprintf(page+len, "%s%u\n", "ifOutOctets=", port_stats->ifOutOctets);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutOctets=", port_stats->ifHCOutOctets);
	len += sprintf(page+len, "%s%u\n", "ifOutUcastPkts=", port_stats->ifOutUcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutUcastPkts=", port_stats->ifHCOutUcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutMulticastPkts=", port_stats->ifOutMulticastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutMulticastPkts=", port_stats->ifHCOutMulticastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutBroadcastPkts=", port_stats->ifOutBroadcastPkts);
	len += sprintf(page+len, "%s%llu\n", "ifHCOutBroadcastPkts=", port_stats->ifHCOutBroadcastPkts);
	len += sprintf(page+len, "%s%u\n", "ifOutDiscards=", port_stats->ifOutDiscards);
	len += sprintf(page+len, "%s%u\n", "ifOutErrors=", port_stats->ifOutErrors);
	len += sprintf(page+len, "%s%u\n", "dot3StatsSingleCollisionFrames=", port_stats->dot3StatsSingleCollisionFrames);
	len += sprintf(page+len, "%s%u\n", "dot3StatsMultipleCollisionFrames=", port_stats->dot3StatsMultipleCollisionFrames);
	len += sprintf(page+len, "%s%u\n", "dot3StatsDefferedTransmissions=", port_stats->dot3StatsDefferedTransmissions);
	len += sprintf(page+len, "%s%u\n", "dot3StatsLateCollisions=", port_stats->dot3StatsLateCollisions);
	len += sprintf(page+len, "%s%u\n", "dot3StatsExcessiveCollisions=", port_stats->dot3StatsExcessiveCollisions);
	len += sprintf(page+len, "%s%u\n", "dot3OutPauseFrames=", port_stats->dot3OutPauseFrames);
	len += sprintf(page+len, "%s%u\n", "dot1dBasePortDelayExceededDiscards=", port_stats->dot1dBasePortDelayExceededDiscards);
	len += sprintf(page+len, "%s%u\n", "etherStatsCollisions=", port_stats->etherStatsCollisions);
	/*here is whole system couters definition*/
	//mark_rm len += sprintf(page+len, "%s%u\n", "dot1dTpLearnedEntryDiscards=", port_stats->dot1dTpLearnedEntryDiscards);
	//mark_rm len += sprintf(page+len, "%s%u\n", "etherStatsCpuEventPkts=", port_stats->etherStatsCpuEventPkts);
	//len += sprintf(page+len, "%s%u\n", "ifInUnknownProtos=", port_stats->ifInUnknownProtos); //mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifAdminStatus=", PortAdminStatus[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifOperStatus=", PortifOperStatus[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifLastChange=", PortLastChange[port]);//mark_rm
	//len += sprintf(page+len, "%s%u\n", "ifSpeed=",port_stats->ifSpeed);
	//len += sprintf(page+len, "%s%u\n", "ifHighSpeed=",port_stats->ifHighSpeed);
	//len += sprintf(page+len, "%s%u\n", "ifCounterDiscontinuityTime=", port_stats->ifCounterDiscontinuityTime);
	/*//mark_rm
	rtl819x_get_port_status(port, &tmp_port_status);
	if(tmp_port_status.link == 1)
		port_stats->ifConnectorPresent = 1;//link
	else
		port_stats->ifConnectorPresent = 2;//unlink
	len += sprintf(page+len, "%s%u\n", "ifConnectorPresent=", port_stats->ifConnectorPresent);
		*/
	
	return len;
}

static int port_mibStats_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{//TODO 
	int len;
	uint32 port;
	struct port_mibStatistics port_stats;
	len = 0;
	port = (uint32)data;
	if(port > CPU)
		return 0;
	rtl819x_get_port_mibStats(port , &port_stats);
	len = show_port_mibStats(&port_stats, port, page, len);

	return len;
}

static int32 port_mibStats_write_proc( struct file *filp, const char *buff,unsigned long len, void *data )
{
	if (len < 2)
		return -EFAULT;

	rtl8651_clearAsicCounter(); 
	return len;
}


#endif 

static int read_proc_eth_drv(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, " \n");

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

#ifdef DBG_EEE
static void eee_chkram(void)
{
	int i;
	uint32 save_reg;

	save_reg = REG32(MDCIOCR);
	REG32(MDCIOCR) = 0x841f0004;
	mdelay(20);

	for (i=0; i <= 127; i++) {         

		REG32(MDCIOCR) = 0x841c0100 + i;
		mdelay(20);
		
		REG32(MDCIOCR) = 0x041d0000 + i;
		mdelay(20);
		
		panic_printk("0xbb804008= 0x%08x (%d)\n", REG32(MDCIOSR), i);
	}
	REG32(MDCIOCR) = save_reg;
}
#endif

static int write_proc_eth_drv(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[80];
	uint32 val;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		char		*strptr, *cmd_addr;
		char		*tokptr;
	
		tmp[count] = '\0';
		strptr=tmp;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL) {
			goto errout;
		}

		if (!memcmp(cmd_addr, "help", 4)) {
			// print help message if supported
			/*
			panic_printk("Usage:\n"
						"  echo dump [asiccounter/mcast] > /proc/eth_drv\n"
						"  echo clear asiccounter > /proc/eth_drv\n"
						"  echo eee [sts/on/off] > /proc/eth_drv\n"
						"  echo phy [read/write] [port id] [page] [register] [value] > /proc/eth_drv\n"
						"  echo mem [read/write] [addr] [value] > /proc/eth_drv\n" );
			*/						
		}
		else if (!memcmp(cmd_addr, "dump", 4))
		{
			// dump table, mib counter
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

#ifdef DBG_ASIC_COUNTER		
			if (!memcmp(tokptr, "asiccounter", 11))
				rtl865xC_dumpAsicCounter(); 
#endif
#ifdef DBG_ASIC_MULTICAST_TBL		
			if (!memcmp(tokptr, "mcast", 5))
				dump_multicast_table(); 
#endif
#ifdef DBG_DESC
			if (!memcmp(tokptr, "desc", 4)) {
				RTL_dumpRxRing(); 
				RTL_dumpTxRing(); 
			}
#endif
		}

#ifdef DBG_SET_CMD
		else if (!memcmp(cmd_addr, "set", 3))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if (!memcmp(tokptr, "time5", 5)) {
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				val=simple_strtoul(tokptr, NULL, 0);
				if ((val >= 150) && (val <= 15000)) 
				{
					total_time_for_5_port = val;
					printk("  set command success. total_time_for_5_port= %d\n", total_time_for_5_port);					
				}
			}
				
		}
#endif		
#ifdef DBG_ASIC_COUNTER		
		else if (!memcmp(cmd_addr, "clear", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if (!memcmp(tokptr, "asiccounter", 11))
				rtl8651_clearAsicCounter(); 
		}
#endif		
#ifdef DBG_EEE
		else if (!memcmp(cmd_addr, "eee", 3)) // eee
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if (!memcmp(tokptr, "on", 2)) {
				eee_enabled = 1;
				printk("  EEE is on.\n");
				eee_phy_enable();

				if (REG32(REVR) == RTL8196C_REVISION_B)
					// enable EEE MAC
					REG32(EEECR) = 0x00739CE7;
			} 
			else if (!memcmp(tokptr, "off", 3)) {
				eee_enabled = 0;
				printk("  EEE is off.\n");
				eee_phy_disable();
				memset (prev_port_sts, 0, sizeof(unsigned char) * MAX_PORT_NUMBER);
				// disable EEE MAC
				REG32(EEECR) = 0x294A5294;
			}
			else if (!memcmp(tokptr, "sts", 3)) {
				printk("  EEE is %s now.\n", (eee_enabled) ? "on" : "off");
			}
			else if (!memcmp(tokptr, "chkram", 6)) {
				eee_chkram();
			}
		}		
#endif		
#ifdef DBG_PHY_REG		
		else if (!memcmp(cmd_addr, "phy", 3))
		{
			uint32 read=1;
			uint32 id, page, reg;
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if ((!memcmp(tokptr, "read", 4)) || (!memcmp(tokptr, "write", 5))){
				if (tokptr[0] == 'w') 
					read = 0;
					
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				if (read && !memcmp(tokptr, "all", 3)) {
					phy_read_all();
					return count;
				}
				// phy id (port id)
				id=simple_strtoul(tokptr, NULL, 0);
				
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				// page
				page=simple_strtoul(tokptr, NULL, 0);

				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				// register number
				reg=simple_strtoul(tokptr, NULL, 0);

				if (read == 0) {
					tokptr = strsep(&strptr," ");
					if (tokptr==NULL)
						goto errout;

					// register value
					val=simple_strtoul(tokptr, NULL, 16);
				}
				phy_op(read, id, page, reg, &val);
			} 
		}
#endif		
#ifdef DBG_MEMORY		
		else if (!memcmp(cmd_addr, "mem", 3)) // eee
		{
			uint32 read=1, k;
			uint32 addr;
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if ((!memcmp(tokptr, "read", 4)) || (!memcmp(tokptr, "write", 5))){
				if (tokptr[0] == 'w') 
					read = 0;
					
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
					goto errout;

				// address
				addr=simple_strtoul(tokptr, NULL, 16);

				if ((addr < 0x80000000) || (addr > 0xbffffffc))
					goto errout;

				tokptr = strsep(&strptr," ");
				
				if ((read == 0) && (tokptr==NULL))
					goto errout;
				
				if (tokptr)
					val=simple_strtoul(tokptr, NULL, 16);
				else 
					val = 16;

				if (read == 0) {
					REG32(addr) = val;
					panic_printk("  write addr= 0x%x, val= 0x%x.\n", addr, val);
				}
				else {
					if (val < 16) val = 16;
					val -= (val % 4);	
					val = val >> 4;

					for (k=0; k<val; k++)
					{
						panic_printk("  0x%x:\t0x%08x\t0x%08x\t0x%08x\t0x%08x\n", addr,
							REG32(addr), REG32(addr+4), REG32(addr+8), REG32(addr+0xc));
						addr += 16;		
					}					
				}
			} 
		}
#endif	
		// success and return
		return count;
errout:
		//panic_printk("  wrong argument.\n");
		return count;
	}
	return -EFAULT;
}

static int read_proc_link_speed(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int i;
	uint32 val;

	sprintf(page, " \n");

	panic_printk("  port \tspeed \tduplex\n");
	panic_printk("=======================\n");
	for(i=0; i<=4; i++)
	{
		val = REG32(PCRP0 + (i * 4));

		if ((val & EnForceMode) == 0)
			panic_printk("  %d    \t%s\n", i, "auto");
		else
			panic_printk("  %d    \t%s    \t%s\n", i,
				(val & ForceSpeed100M) ? "100" : "10",
				(val & ForceDuplex) ? "full" : "half");					
	}

	*eof = 1;
	*start = page + off;
	return 0;
}

const uint32 mac_speed[5] = {	0x007f0000, 0x03870000, 0x038f0000, 0x03830000, 0x038b0000 };

const uint16 phy_speed[5] = {
	ENABLE_AUTONEGO,
	SELECT_FULL_DUPLEX,
	SPEED_SELECT_100M | SELECT_FULL_DUPLEX,
	0,
	SPEED_SELECT_100M,
};

static int write_proc_link_speed(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[80];

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, count)) {

		char		*strptr, *cmd_addr;
		char		*tokptr;
		int		sport = -1, eport, speed = -1, i;
	
		tmp[count] = '\0';
		strptr=tmp;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL) {
			goto errout;
		}

		if (!memcmp(cmd_addr, "help", 4)) {
			// print help message if supported
			/*
			panic_printk("Usage:\n"
						"  echo port (0~4/all) speed (10/100/auto) [duplex (full/half)] > /proc/link_speed\n"
						);
			*/						
		}
		else if (!memcmp(cmd_addr, "all", 3))			
		{
			sport = 0;
			eport = 4;
		}		
		else if ((cmd_addr[0] >= '0') && (cmd_addr[0] <= '4'))			
		{
			sport = eport = cmd_addr[0] - '0';
		}		

		if (sport >= 0) {
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
				goto errout;

			if (!memcmp(tokptr, "100", 3))
				speed = 2;
			else if (!memcmp(tokptr, "10", 2))
				speed = 1;
			else if (!memcmp(tokptr, "auto", 4))
				speed = 0;
		}			

		if (speed > 0) {
			tokptr = strsep(&strptr," ");
			if (tokptr && (!memcmp(tokptr, "half", 4))) {
				speed += 2;
			}
		}			

		if (sport >= 0) {
			uint32 reg, val;

			for(i=sport; i<=eport; i++)
			{
				reg = PCRP0 + (i * 4);
				REG32(reg) = (REG32(reg) & (~0x03FF0000)) | mac_speed[speed];			
			
				rtl8651_getAsicEthernetPHYReg( i, 0, &val);
				val = (val & (~0x3140)) | phy_speed[speed];
				rtl8651_setAsicEthernetPHYReg( i, 0, val);

				rtl8651_restartAsicEthernetPHYNway(i+1, i);							
			}
		}

		// success and return
		return count;
errout:
		//panic_printk("  wrong argument.\n");
		return count;
	}
	return -EFAULT;
}

#endif

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
static struct proc_dir_entry *proc_disable_L2=NULL;

static int read_proc_disable_L2(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", L2_table_disabled);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}

static int write_proc_disable_L2(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[4];

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 2)) {

		L2_table_disabled = tmp[0] - '0';
		if (L2_table_disabled)
			L2_table_disabled = 1;
#ifndef CONFIG_RTL865X_HW_TABLES
		SoftNAT_OP_Mode(savOP_MODE_value);
#endif				
		return count;
	}
	return -EFAULT;
}
#endif

#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
static struct proc_dir_entry *proc_port_speed=NULL;

static int write_proc_port_speed(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[4];
	uint32 statCtrlReg, phyId; // = wan_port;

	if (count < 2)
		return -EFAULT;

	if (buffer && !copy_from_user(tmp, buffer, 3)) {
		/* tmp[0]: '0' -- auto,	   '8' -- 10M
				   '9' -- 100M,	   'a' -- Giga
		    tmp[2]: port number
		 */
		phyId = tmp[2] - '0'; 
		rtl8651_getAsicEthernetPHYReg( phyId, 4, &statCtrlReg );
		statCtrlReg &= ~(CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
		if (tmp[0] == '8')
			statCtrlReg |= (CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
		else if (tmp[0] == '9')
			statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD);
		else
			statCtrlReg |= (CAPABLE_100BASE_TX_FD | CAPABLE_100BASE_TX_HD | CAPABLE_10BASE_TX_FD | CAPABLE_10BASE_TX_HD);
			
		rtl8651_setAsicEthernetPHYReg( phyId, 4, statCtrlReg );
		rtl8651_getAsicEthernetPHYReg( phyId, 0, &statCtrlReg );

		statCtrlReg |= RESTART_AUTONEGO;
		rtl8651_setAsicEthernetPHYReg( phyId, 0, statCtrlReg );				

		return count;
	}
	return -EFAULT;
}
#endif

#endif

#if defined(CONFIG_HW_MULTICAST_TBL) || defined(CONFIG_RTK_VLAN_SUPPORT)||defined (CONFIG_RTL_HARDWARE_MULTICAST)
/* Chip Version */
#define RTL865X_CHIP_VER_RTL865XB	0x01
#define RTL865X_CHIP_VER_RTL865XC	0x02
#define RTL865X_CHIP_VER_RTL8196B	0x03

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
extern int32 RtkHomeGatewayChipRevisionID;
extern int32 RtkHomeGatewayChipNameID;
#else
int32 RtkHomeGatewayChipRevisionID;
int32 RtkHomeGatewayChipNameID;
#endif
#endif

#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL


static int isCreated;
static struct proc_dir_entry *res_custom_passthru=NULL;

static int custom_Passthru_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	int flag,i;

	if (buffer && !copy_from_user(passThru_flag, buffer, 1))
	{			
		//flag = passThru_flag[0] - '0';
		flag= passThru_flag[0] ;
		if(flag ^ old_passThru_flag)
		{			
			//IPv6 PassThru
			if((flag & IP6_PASSTHRU_MASK) ^ (old_passThru_flag & IP6_PASSTHRU_MASK)) 
			{			
				if(flag & IP6_PASSTHRU_MASK)
				{//add				
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, TRUE, PASSTHRU_VLAN_ID);
					}				
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}
			#if 1
			//PPPoE PassThru
			if((flag & PPPOE_PASSTHRU_MASK) ^ (old_passThru_flag & PPPOE_PASSTHRU_MASK)) 
			{			
				if(flag & PPPOE_PASSTHRU_MASK)
				{//add				
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, TRUE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, TRUE, PASSTHRU_VLAN_ID);
					}				
				}
				else
				{//delete
					for(i=0; i<RTL865XC_PORT_NUMBER; i++)
					{
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_CONTROL, i, FALSE, PASSTHRU_VLAN_ID);
						rtl8651_setProtocolBasedVLAN(RTL8651_PBV_RULE_PPPOE_SESSION, i, FALSE, PASSTHRU_VLAN_ID);
					}
				}
			}
			#endif
			//vlan
			if(!isCreated)
			{
				rtl_addRtlVlanEntry(PASSTHRU_VLAN_ID);
			#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
			rtl_addRtlVlanMemPort(PASSTHRU_VLAN_ID, RTL_LANPORT_MASK|RTL_WANPORT_MASK|0x100);
			#else
			rtl_addRtlVlanMemPort(PASSTHRU_VLAN_ID, RTL_LANPORT_MASK|RTL_WANPORT_MASK);
			#endif
				//passthru_vlan_create();
				//isCreated=1;
			}
		}
		old_passThru_flag=flag;
		return count;
	}
	return -EFAULT;
}

static int custom_Passthru_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;	
	len = sprintf(page, "%c\n", passThru_flag[0]);
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}




void updateProtocolBasedVLAN(void)
{
	int i, _add;

	_add = (oldStatus & IP6_PASSTHRU_MASK) ? TRUE : FALSE;
	
	for(i=0; i<RTL865XC_PORT_NUMBER; i++)
		rtl8651_setProtocolBasedVLAN(IP6_PASSTHRU_RULEID, i, _add, PASSTHRU_VLAN_ID);
	
	passthru_vlan_create();
}


#endif
#endif /* CONFIG_PROC_FS */

//---------------------------------------------------------------------------
#ifdef __KERNEL__
static int __init rtl865x_probe(int ethno)
#else
void *rtl865x_probe(int ethno)
#endif
{
#if defined (CONFIG_RTL_IGMP_SNOOPING)
		int32 retVal;
		int32 igmpInitFlag=FAILED;
		struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig;
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		rtl865x_mCastConfig_t mCastConfig;
	#endif
#endif

#ifdef DYNAMIC_ADJUST_TASKLET
	struct proc_dir_entry *res1;
#endif	

#ifdef __KERNEL__
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL8196C_EC)
	struct proc_dir_entry *res_stats_root;
	struct proc_dir_entry *res_stats;
#endif

#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	struct proc_dir_entry *res_stats_root;
	struct proc_dir_entry *res_stats;
#endif

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	struct proc_dir_entry *res2;
#endif
#endif

#ifdef MODULE
	printk("%s", version);
#endif

	struct net_device *dev;
	struct re_private *cp;
	void *regs;
	unsigned i;
#ifdef __KERNEL__	
	int rc;
	struct proc_dir_entry *res;
#endif

#if defined(CONFIG_RTL_8197F)
	memset(sw_pvid, 0, sizeof(int)*RTL8651_PORT_NUMBER);
#endif

#ifndef MODULE
	static int version_printed;
	if (version_printed++ == 0)
		printk("%s", version);
#endif

#if defined(CONFIG_RTL_8196E)
	if ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES) 
		bonding_type = BOND_8196ES;
#ifdef CONFIG_RTL_ALP
	else if((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196E) 
		bonding_type = BOND_8196E;
#endif
	else if ( ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196EU)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196EU1)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196EU2)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196EU3)  ){
		bonding_type = BOND_8196EU;
	}
#endif

#if defined(CONFIG_RTL_8881A)
#if defined(CONFIG_RTL_8881AM) || (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH))
    if ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8881AM) {
        bonding_type = BOND_8881AM;
    }
    if ((ethno == 0) && (bonding_type==BOND_8881AM)) { 		// Check is RTL8881AM 
        REG32(PIN_MUX_SEL) = (REG32(PIN_MUX_SEL)&~(0x7<<7))|(0x3<<7); //Set MUX to GPIO
        REG32(PEFGH_CNR) = REG32(PEFGH_CNR) & ~(0x80); //Set E7 for  Gpio
        REG32(PEFGH_DIR) = REG32(PEFGH_DIR) & ~(0x80); //Set E7 for Input Mode
        if((REG32(PEFGH_DAT)&0x80) == 0x80) { //Pull high
            cmj_board_use_port4 = 1;
            wan_port = 4;

            #define EPHY_CONTROL		0xb800009c
            #define EN_ROUTER_MODE	(1<<12)	//1'b1: 5-port router mode(default); 1'b0: 1-port AP mode
            REG32(EPHY_CONTROL) &= ~EN_ROUTER_MODE;
        }
        else {
            cmj_board_use_port4 = 0; // hardware board use port 1
        }
    }		
#endif
#endif

#if defined(CONFIG_RTL_8197F)
	#if !defined(CONFIG_RTL_8367R_SUPPORT) && !defined(CONFIG_RTL_8211F_SUPPORT)
	{
		int32 totalVlans=((sizeof(vlanSetting))/(sizeof(struct  init_vlan_setting)))-1;
		extern uint32 rtl819x_bond_option(void);
		if (rtl819x_bond_option() == 3){
			//rtl865x_wanPortMask = 0;
			//rtl865x_lanPortMask = 0x110;
			for(i=0;i<totalVlans;i++)
			{
				if (vlanSetting[i].is_wan == 0) {
					vlanSetting[i].portmask = 0x110;
				}
				else {
					vlanSetting[i].portmask = 0;
				}
			}
		}
	}		
	#endif
#endif

	if (ethno == 0) {
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
		FullAndSemiReset();
#endif
		chip_id = (READ_MEM32(CVIDR)) >> 16; 
		if (chip_id == 0x8196) 
			chip_revision_id = (READ_MEM32(CVIDR)) & 0x0f;
		else
			chip_revision_id = ((READ_MEM32(CRMR)) >> 12) & 0x0f;

		#ifdef CONFIG_HW_MULTICAST_TBL
		if (chip_id == 0x8196) 
			RtkHomeGatewayChipNameID = RTL865X_CHIP_VER_RTL8196B;
		else
			RtkHomeGatewayChipNameID = RTL865X_CHIP_VER_RTL865XC;
		
		RtkHomeGatewayChipRevisionID = chip_revision_id;
		#endif        

		#ifndef __KERNEL__	
		#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
		WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
		#endif		
		#endif        
	}

#ifdef CONFIG_RTL865X_HW_TABLES
	if (ethno == 0) { 
		// be initialized once only
		FullAndSemiReset();
		rtl8651_extDeviceinit();
		INIT_CHECK(rtl865x_lightRomeInit()); 
		INIT_CHECK(rtl865x_lightRomeConfig());
		rtl8651_extDeviceInitTimer(); 
	}
#endif    

	if(ethno == 0){
#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
    		REG32(CPUICR1) = (REG32(CPUICR1) & ~CF_PKT_HDR_TYPE_MASK) | TX_PKTHDR_SHORTCUT_LSO;
#endif
		/*Init hw table*/
 #ifdef CONFIG_RTL_LAYERED_DRIVER
 #if 0//defined(CONFIG_RTL_8197FS)
 #if defined(CONFIG_RTL_HARDWARE_MULTICAST)
 		rtl8651_setAsicOperationLayer(3);
 #else
 		rtl8651_setAsicOperationLayer(2);
 #endif
 #else
 		rtl8651_setAsicOperationLayer(4);
 #endif
	 	rtl865x_initNetifTable();
 #endif
 		rtl865x_initEventMgr(NULL);
		/*l2*/
 #ifdef CONFIG_RTL_LAYERED_DRIVER_L2
		rtl865x_layer2_init();
 #endif

 #ifdef CONFIG_RTL_LAYERED_DRIVER_L3
 		WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
		//Enable TTL-1
		WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.

 		rtl865x_initIpTable();
		rtl865x_initPppTable();
		rtl865x_initRouteTable();
		rtl865x_initNxtHopTable();
		rtl865x_arp_init();
 #endif
 
 #ifdef CONFIG_RTL_LAYERED_DRIVER_L4
 		rtl865x_initAsicL4();
 		rtl865x_nat_init();
#endif

		#if defined(CONFIG_RTL_VLAN_SUPPORT) ||defined(CONFIG_RTL_8197F)
		//port based decision
		rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
		WRITE_MEM32(PLITIMR,0);
		#if defined(CONFIG_RTL_VLAN_SUPPORT)
		rtl_initRtlVlan();
		#endif
		#endif
		
		#if defined(CONFIG_RTL_CONE_NAT_SUPPORT)
		rtl_initRtlConeNat();
		#endif

		#if defined(CONFIG_RTL_PORT_FORWARDING_SUPPORT)
		rtl_initRtlPortFwd();
		#endif

		#if defined(CONFIG_RTL_TRIGGER_PORT_SUPPORT)
		rtl_initTriggerPort();
		#endif
        
		#if defined (CONFIG_RTL_IGMP_SNOOPING)
		
		memset(&mCastSnoopingGlobalConfig, 0, sizeof(struct rtl_mCastSnoopingGlobalConfig));
			
		mCastSnoopingGlobalConfig.groupMemberAgingTime=260;
		mCastSnoopingGlobalConfig.lastMemberAgingTime=2;
		mCastSnoopingGlobalConfig.querierPresentInterval=260;
	
		mCastSnoopingGlobalConfig.dvmrpRouterAgingTime=120;
		mCastSnoopingGlobalConfig.mospfRouterAgingTime=120;
		mCastSnoopingGlobalConfig.pimRouterAgingTime=120;
	
		igmpInitFlag=rtl_initMulticastSnooping(mCastSnoopingGlobalConfig);
		
		#endif
	}

	dev = alloc_etherdev(sizeof(struct re_private));
	if (!dev)
#ifdef __KERNEL__
		return -ENOMEM;
#else
		return NULL;
#endif
		
	SET_MODULE_OWNER(dev);
	cp = dev->priv;
	cp->dev = dev;
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    if (ethno != 6)//eth7 bridge vlan virtual interface
    #endif
	    reNet[ethno] = dev;
	if (ethno == 1) {
		cp->dev_prev = reNet[0];
		((struct re_private *)reNet[0]->priv)->dev_next = dev;
	}
			
	spin_lock_init(&cp->lock);

	 /* Set Default MAC address */
	for (i = 0; i < 6; i++)
		((u8 *)(dev->dev_addr))[i] =vlanSetting[ethno].mac[i];

	cp->vid = vlanSetting[ethno].vid;
	cp->port = vlanSetting[ethno].portmask;
	#if defined(CONFIG_RTL_819X)	//jwj:20120821
	cp->is_wan = vlanSetting[ethno].is_wan;
	#endif
	
	regs = (void *)((ethno) ? SWMACCR_BASE : SWMACCR_BASE);
	dev->base_addr = (unsigned long) regs;
	dev->irq = CYGNUM_HAL_INTERRUPT_SWCORE; //ICU_NIC

	dev->open = rtl865x_open;
	dev->stop = rtl865x_close;
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    if (ethno == 6)
    {
        //diag_printf("%s %d init rtl_bridge_wan_start_xmit\n", __FUNCTION__, __LINE__);
        dev->hard_start_xmit = rtl_bridge_wan_start_xmit;
    }
    else
    #endif
	dev->hard_start_xmit = rtl865x_start_xmit;
	dev->do_ioctl = re865x_ioctl;
	dev->get_stats = rtl865x_get_stats;
#ifdef __KERNEL__
	dev->set_mac_address = rtl865x_set_hwaddr;
#endif

#ifdef __ECOS
	dev->isr = interrupt_isr;	
	dev->dsr= interrupt_dsr;
	dev->can_xmit = can_xmit;
	if (ethno == 0)
		strcpy(dev->name, "eth0");
	else if (ethno == 1)
		strcpy(dev->name, "eth1");
#ifdef CONFIG_RTL_VLAN_SUPPORT
	else if (ethno == 2)
		strcpy(dev->name, "eth2");
	else if (ethno == 3)
		strcpy(dev->name, "eth3");
	else if (ethno == 4)
		strcpy(dev->name, "eth4");
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    else if (ethno == 6)
    {
        strcpy(dev->name, "eth7");
        pVirtualDev = dev;        
        //diag_printf("%s %d init pVirtualDev\n", __FUNCTION__, __LINE__);
    }
#endif    
#endif
#endif
	//diag_printf("dev:%s,vid:%x,port:%x[%s]:[%d].\n",cp->dev->name,cp->vid,cp->port,__FUNCTION__,__LINE__);
	if (ethno == 0) {

#if defined (CONFIG_RTL_IGMP_SNOOPING)
		retVal=rtl_registerIgmpSnoopingModule(&nicIgmpModuleIndex);
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(retVal==SUCCESS)
		{
			rtl_multicastDeviceInfo_t devInfo;
			memset(&devInfo, 0 , sizeof(rtl_multicastDeviceInfo_t));
			strcpy(devInfo.devName, "eth*");
			#if 0
			for(i=0;i<totalVlans;i++)
			{
				if( vlanconfig[i].if_type==IF_ETHER)
				{
					devInfo.portMask|=vlanconfig[i].memPort;
				}
			}
			#else
			devInfo.portMask|=0xff;
			#endif
			devInfo.swPortMask=devInfo.portMask & (~ ((1<<RTL8651_MAC_NUMBER)-1));
			rtl_setIgmpSnoopingModuleDevInfo(nicIgmpModuleIndex, &devInfo);
		}
		#endif
		rtl_setIpv4UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);
#ifdef CONFIG_RTL_MLD_SNOOPING		
		rtl_setIpv6UnknownMCastFloodMap(nicIgmpModuleIndex, 0xFFFFFFFF);
#endif
		#if 1 //def CONFIG_RTL_8197D_DYN_THR
		curLinkPortMask=rtl865x_getPhysicalPortLinkStatus();
		#endif
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		
		memset(&mCastConfig, 0, sizeof(rtl865x_mCastConfig_t));
		#if 0
		for(i=0;i<totalVlans;i++)
		{
			if (TRUE==vlanconfig[i].isWan)
			{
				mCastConfig.externalPortMask |=vlanconfig[i].memPort;
			}
		}
		#else
		
		mCastConfig.externalPortMask=0x0;
		
		rtl865x_initMulticast(&mCastConfig);
		#endif
		rtl8651_setAsicMulticastEnable(TRUE);
		#else
		//rtl8651_setAsicMulticastEnable(FALSE);
		#endif
		
#endif
	}
#ifdef CP_TX_CHECKSUM
	dev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
#endif

#ifdef __KERNEL__	
	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;
#endif
	printk(KERN_INFO "%s: %s at 0x%lx, "
		"%02x:%02x:%02x:%02x:%02x:%02x, "
		"IRQ %d\n",
		dev->name,
		"RTL865x-NIC",
		dev->base_addr,
		dev->dev_addr[0], dev->dev_addr[1],
		dev->dev_addr[2], dev->dev_addr[3],
		dev->dev_addr[4], dev->dev_addr[5],
		dev->irq);

#ifdef CONFIG_PROC_FS
	if (ethno == 0) {
		res = create_proc_entry("eth_flag", 0, NULL);
    		if (res)
			res->write_proc = write_proc;
    
		#ifdef DYNAMIC_ADJUST_TASKLET
		res1 = create_proc_entry("rx_pkt_thres", 0, NULL);
		if (res1)
			res1->write_proc = write_proc_rxthres;
		#endif      

		#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		if ((res2 = create_proc_read_entry("eth_stats", 0644, NULL,
			read_proc_eth_stats, (void *)dev)) == NULL) {
			printk("create_proc_read_entry failed!\n");
			goto err_out_iomap;
		}
		#endif
	}
	
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL8196C_EC)
	res_stats_root = proc_mkdir(dev->name, NULL);
	if (res_stats_root == NULL) {
		printk("proc_mkdir failed!\n");
		goto err_out_iomap;
	}
	
#ifdef CONFIG_RTK_VLAN_SUPPORT
	if ((res_stats = create_proc_read_entry("mib_vlan", 0644, res_stats_root,
			read_proc_vlan, (void *)dev)) == NULL) {
		printk("create_proc_read_entry failed!\n");
		goto err_out_iomap;
	}
	res_stats->write_proc = write_proc_vlan;	
#endif	

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if ((res_stats = create_proc_read_entry("stats", 0644, res_stats_root,
		read_proc_stats, (void *)dev)) == NULL) {
		printk("create_proc_read_entry failed!\n");
		goto err_out_iomap;
	}
	res_stats->write_proc = write_proc_stats;
#endif
#endif

#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		if (ethno == 1) { 
	#endif
	res_stats_root = proc_mkdir(dev->name, NULL);
	if (res_stats_root == NULL) {
		printk("proc_mkdir failed!\n");
		goto err_out_iomap;
	}
	if ((res_stats = create_proc_read_entry("stats", 0644, res_stats_root,
		read_proc_stats, (void *)dev)) == NULL) {
		printk("create_proc_read_entry failed!\n");
		goto err_out_iomap;
	}
	res_stats->write_proc = write_proc_stats;
	
	#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	} 
	#endif
	
#endif
#endif // CONFIG_PROC_FS

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	if (ethno == 0) { 
		init_priv_eth_skb_buf();
	}
#endif		
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#ifdef CONFIG_RTL_VLAN_SUPPORT
	if (ethno == 4)
#else
	if (ethno == 1)
#endif
	{
		rtl8651_customPassthru_init();
	}
#endif

#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
	if (ethno == 0) { 
		rtl8651_customPassthru_init();
	}
#endif

#ifdef __KERNEL__
	return 0;

err_out_iomap:
	printk("in err_out_iomap\n");
	iounmap(regs);
	kfree(dev);
	return -1 ;
#else
	return (void *)dev;
#endif
}
struct net_device *rtl_get_peth0_net_device(int device_num)
{
	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	if (pDev){
		return pDev;
	}	
	else {
		
		rtl8651_customPassthru_init();
		
		if (pDev)
			return pDev;
		else
			return NULL;
	}	
	#else 
		return NULL;
	#endif
}

//---------------------------------------------------------------------------
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
#define ETH_SKB_BUF_SIZE	(CROSS_LAN_MBUF_LEN+sizeof(struct skb_shared_info)+128)
#define ETH_MAGIC_CODE		"865x"

struct priv_skb_buf2 {
	unsigned char magic[4];
	unsigned int buf_pointer;		
	struct list_head	list;	
	unsigned char buf[ETH_SKB_BUF_SIZE];
};
#ifdef CONFIG_RTL8196B
struct priv_skb_buf2 eth_skb_buf[MAX_ETH_SKB_NUM];
#else
static struct priv_skb_buf2 eth_skb_buf[MAX_ETH_SKB_NUM];
#endif
static struct list_head eth_skbbuf_list;

extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);

//---------------------------------------------------------------------------
static void init_priv_eth_skb_buf(void)
{
	int i;

	memset(eth_skb_buf, '\0', sizeof(struct priv_skb_buf2)*MAX_ETH_SKB_NUM);
	INIT_LIST_HEAD(&eth_skbbuf_list);
	
	for (i=0; i<MAX_ETH_SKB_NUM; i++)  {
		memcpy(eth_skb_buf[i].magic, ETH_MAGIC_CODE, 4);	
		eth_skb_buf[i].buf_pointer = (unsigned int)&eth_skb_buf[i];				
		INIT_LIST_HEAD(&eth_skb_buf[i].list);
		list_add_tail(&eth_skb_buf[i].list, &eth_skbbuf_list);	
        
	}
}

//---------------------------------------------------------------------------
static __inline__ unsigned char *get_buf_from_poll(struct list_head *phead, unsigned int *count)
{
	unsigned char *buf;
	struct list_head *plist;

	if (list_empty(phead)) {
//		DEBUG_ERR("eth_drv: phead=%X buf is empty now!\n", (unsigned int)phead);
		return NULL;
	}

	if (*count == 0) {
//		DEBUG_ERR("eth_drv: phead=%X under-run!\n", (unsigned int)phead);
		return NULL;
	}

	*count = *count - 1;
	plist = phead->next;
	list_del_init(plist);
	buf = (unsigned char *)((unsigned int)plist + sizeof (struct list_head));
	return buf;
}

//---------------------------------------------------------------------------
static __inline__ void release_buf_to_poll(unsigned char *pbuf, struct list_head	*phead, unsigned int *count)
{
	struct list_head *plist;

	*count = *count + 1;
	plist = (struct list_head *)((unsigned int)pbuf - sizeof(struct list_head));
	list_add_tail(plist, phead);
}

//---------------------------------------------------------------------------
void free_rtl865x_eth_priv_buf(unsigned char *head)
{
	unsigned long flags;
	#ifdef DELAY_REFILL_ETH_RX_BUF
	extern int return_to_rx_pkthdr_ring(unsigned char *head);
#ifdef ETH_NEW_FC
	if (during_close || !return_to_rx_pkthdr_ring(head)) 
#else
	if (!return_to_rx_pkthdr_ring(head)) 
#endif
	#endif
	{
		save_and_cli(flags);	
		release_buf_to_poll(head, &eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);	
		restore_flags(flags);
	}
}

//---------------------------------------------------------------------------
static struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size)
{
	struct sk_buff *skb;
	unsigned char *data;
	unsigned long flags;

	/* first argument is not used */
	save_and_cli(flags);
	data = get_buf_from_poll(&eth_skbbuf_list, (unsigned int *)&eth_skb_free_num);
	restore_flags(flags);

	if (data == NULL) {
//		DEBUG_ERR("eth_drv: priv skb buffer empty!\n");
		return NULL;
	}
	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		free_rtl865x_eth_priv_buf(data);
		return NULL;
	}
    
	return skb;
}

//---------------------------------------------------------------------------
int is_rtl865x_eth_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf2 *)0)->buf);
	struct priv_skb_buf2 *priv_buf = (struct priv_skb_buf2 *)(((unsigned long)head) - offset);

	if ((!memcmp(priv_buf->magic, ETH_MAGIC_CODE, 4)) &&
		(priv_buf->buf_pointer == (unsigned int)priv_buf)) {		
		return 1;	
	}
	else {
		return 0;
	}
}

//---------------------------------------------------------------------------
#ifdef CONFIG_NET_RADIO
#ifdef __KERNEL__
#include <net/dst.h>
static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	/*
	 *	Shift between the two data areas in bytes
	 */
	unsigned long offset = new->data - old->data;

	new->list=NULL;
	new->sk=NULL;
	new->dev=old->dev;
	new->priority=old->priority;
	new->protocol=old->protocol;
	new->dst=dst_clone(old->dst);
	new->h.raw=old->h.raw+offset;
	new->nh.raw=old->nh.raw+offset;
	new->mac.raw=old->mac.raw+offset;
	memcpy(new->cb, old->cb, sizeof(old->cb));
	atomic_set(&new->users, 1);
	new->pkt_type=old->pkt_type;
	new->stamp=old->stamp;
	new->destructor = NULL;
	new->security=old->security;
#ifdef CONFIG_NETFILTER
	new->nfmark=old->nfmark;
	new->nfcache=old->nfcache;
	new->nfct=old->nfct;
	nf_conntrack_get(new->nfct);
#ifdef CONFIG_NETFILTER_DEBUG
	new->nf_debug=old->nf_debug;
#endif
#endif
#ifdef CONFIG_NET_SCHED
	new->tc_index = old->tc_index;
#endif
#ifdef CONFIG_RTK_VOIP_VLAN_ID
	new->rx_vlan=old->rx_vlan;
	new->rx_wlan=old->rx_wlan;
#endif
#ifdef CONFIG_RTK_VLAN_SUPPORT	
	new->tag.v = old->tag.v;
#endif
}
#endif

//---------------------------------------------------------------------------
struct sk_buff *priv_skb_copy(struct sk_buff *skb)
{
	struct sk_buff *n;	

#ifdef RTK_QUE
	n = rtk_dequeue(&rx_skb_queue);
#else
	n = skb_dequeue(&rx_skb_queue);
#endif

	if (n == NULL) {
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
		n = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
#else        
		n  = dev_alloc_skb(CROSS_LAN_MBUF_LEN);
#endif
		if (n == NULL)
			return NULL;
	}

	/* Set the tail pointer and length */	
	skb_put(n, skb->len);	

#ifdef __KERNEL__	
	n->csum = skb->csum;	
	n->ip_summed = skb->ip_summed;	
#endif

	memcpy(n->data, skb->data, skb->len);

	copy_skb_header(n, skb);
	return n;
}
#endif // CONFIG_NET_RADIO
#endif // CONFIG_RTL865X_ETH_PRIV_SKB

#ifdef __KERNEL__
//---------------------------------------------------------------------------
static void __exit rtl865x_exit (void)
{
}

//---------------------------------------------------------------------------
static int __init rtl865x_init(void)
{ 
#ifdef CONFIG_819X_PHY_RW
	uint32 portnum;
	char port_mibEntry_name[10];	
#endif
#ifdef CONFIG_PROC_FS
	proc_wan_port = create_proc_entry("wan_port", 0, NULL);
	if (proc_wan_port) {
		proc_wan_port->read_proc = read_proc_wan_port;
		proc_wan_port->write_proc = write_proc_wan_port;
	}  

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
	proc_eth_drv = create_proc_entry("eth_drv", 0, NULL);
	if (proc_eth_drv) {
		proc_eth_drv->read_proc = read_proc_eth_drv;
		proc_eth_drv->write_proc = write_proc_eth_drv;
	}  
	proc_link_speed = create_proc_entry("link_speed", 0, NULL);
	if (proc_link_speed) {
		proc_link_speed->read_proc = read_proc_link_speed;
		proc_link_speed->write_proc = write_proc_link_speed;
	}  
#if defined(CONFIG_819X_PHY_RW)
	rtl_phy = create_proc_entry("rtl_phy_status",0,NULL);
	if(rtl_phy)
	{
		rtl_phy->read_proc = rtl_phy_status_read;
		rtl_phy->write_proc = rtl_phy_status_write;
	}

	port_mibStats_root = proc_mkdir("ethPort_mibStats", NULL);
	if (port_mibStats_root == NULL) 
	{
		printk("proc_mkdir failed!\n");		
	}
	for(portnum=0; portnum<CPU; portnum++)
	{
		sprintf(&port_mibEntry_name[0], "port%u", portnum);
		port_mibStats_entry = create_proc_entry(port_mibEntry_name, 0, port_mibStats_root);
		port_mibStats_entry->data = (void *)portnum;
		if(port_mibStats_entry)
		{
			port_mibStats_entry->read_proc = port_mibStats_read_proc;
			port_mibStats_entry->write_proc = port_mibStats_write_proc;
		}
	}
#endif	
#endif

	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	proc_disable_L2 = create_proc_entry("disable_l2_table", 0, NULL);
	if (proc_disable_L2) {
		proc_disable_L2->read_proc = read_proc_disable_L2;
		proc_disable_L2->write_proc = write_proc_disable_L2;
	}  
	#endif

	#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	proc_port_speed = create_proc_entry("port_speed", 0, NULL);
	if (proc_port_speed) {
		proc_port_speed->write_proc = write_proc_port_speed;
	}  
	#endif
#endif

#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTK_VOIP_DRIVERS_WAN_PORT_4) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
	wan_port = 4; 

	/* set LAN/WAN port mask */
	vlanSetting[0].portmask = ALL_PORTS & (~BIT(wan_port));
	vlanSetting[1].portmask=BIT(wan_port);
		
	#ifdef CONFIG_RTL865X_HW_TABLES
	lrConfig[0].memPort = ALL_PORTS & (~BIT(wan_port));
	lrConfig[0].untagSet = lrConfig[0].memPort;
	lrConfig[1].memPort = BIT(wan_port);
	lrConfig[1].untagSet = lrConfig[1].memPort;
	#ifndef CONFIG_RTL865X_HW_PPTPL2TP
	lrConfig[2].memPort = BIT(wan_port);
	lrConfig[2].untagSet = lrConfig[2].memPort;
	#endif
	#endif

#elif defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_8196CS) || defined(CONFIG_RTL_ETH_PORT4_ONLY)
	wan_port = PORT_HW_AP_MODE; 

	/* set LAN/WAN port mask */
	vlanSetting[0].portmask = 0;
	vlanSetting[1].portmask=BIT(wan_port);
		
#else
	wan_port = 0; 
#endif

#ifdef CONFIG_RTL8196_RTL8366	
	{
		int ret;
		int i;
		rtl8366rb_phyAbility_t phy;
		REG32(PEFGHCNR_REG) = REG32(PEFGHCNR_REG)& (~(1<<11) ); //set byte F GPIO3 = gpio
	        REG32(PEFGHDIR_REG) = REG32(PEFGHDIR_REG) | (1<<11);  //0 input, 1 output, set F bit 3 output
		REG32(PEFGHDAT_REG) = REG32(PEFGHDAT_REG) |( (1<<11) ); //F3 GPIO
		mdelay(150);
		ret = smi_init(GPIO_PORT_F, 2, 1);
		
		ret = rtl8366rb_initChip();
		ret = rtl8366rb_initChip();
		ret = rtl8366rb_initVlan();
		ret = rtl8366rb_initAcl();
		ret = smi_write(0x0f09, 0x0020);
		ret = smi_write(0x0012, 0xe0ff);

		memset(&phy, 0, sizeof(rtl8366rb_phyAbility_t));
		phy.Full_1000 = 1;
		phy.Full_100 = 1;
		phy.Full_10 = 1;
		phy.Half_100 = 1;
		phy.Half_10 = 1;
		phy.FC = 1;
		phy.AsyFC = 1;
		phy.AutoNegotiation = 1;
		for(i=0;i<5;i++)
		{
			ret = rtl8366rb_setEthernetPHY(i,&phy);
		}
		//ret = smi_write(0x0012, 0xe0ff);

		REG32(0xb8010000)=REG32(0xb8010000)&(0x20000000);
		REG32(0xbb80414c)=0x00037d16;
		REG32(0xbb804100)=1;
		REG32(0xbb804104)=0x00E80367;
	}
#endif


	rtl865x_probe(0);
	rtl865x_probe(1);

	#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	rtl865x_probe(2); // eth2
	rtl865x_probe(3); // eth3
	rtl865x_probe(4); // eth4

#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT) && defined(CONFIG_RTK_VLAN_ROUTETYPE)
	rtl865x_probe(5); // eth5
	rtl865x_probe(6); // eth6
#elif defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT) || defined(CONFIG_RTK_VLAN_ROUTETYPE)
	rtl865x_probe(5); // eth5
#endif

#else //#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	
#if defined(CONFIG_RTK_IPV6_PASSTHRU_SUPPORT)
	rtl865x_probe(2); // eth2
#endif	
	
#endif //#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)


	
#ifdef PREVENT_BCAST_REBOUND
	memset( &bcast , 0,  sizeof(struct bcast_tr_s ));
#endif

#ifdef CONFIG_RTL8196C_ETH_LED_BY_GPIO
	WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
#endif
	return 0;
}
#endif /* __KERNEL__ */

#ifdef CONFIG_RTL8196_RTL8366
void force_reset_nic_8366()
{
	{
                int ret;
		delay_ms(200);
                ret = rtl8366rb_initChip();
               	ret = rtl8366rb_initChip();
		ret = rtl8366rb_initVlan();

         }
}
#endif
//---------------------------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_WAN_VLAN
void add_WAN_VLAN(unsigned int nvid){
	int ret;
	rtl_vlan_param_t vp;
	bzero((void *) &vp, sizeof(rtl_vlan_param_t));
	
	vp.egressUntag = vlanSetting[1].portmask; //vlanSetting[1].portmask;
	vp.memberPort = vlanSetting[1].portmask;
#ifdef CONFIG_RTL865X_HW_TABLES
	vp.fid = 1;
#else
	vp.fid = 1;
#endif

	ret = swCore_vlanCreate(nvid, &vp);  //P1-P4
	if ( ret != 0 )
		printk("865x-nic: swCore_vlanCreate(%d) failed:%d\n", nvid, ret );
}

void del_WAN_VLAN(unsigned short nvid){
	if( nvid != vlanSetting[1].vid && nvid != vlanSetting[0].vid)
		swCore_vlanDestroy(nvid);
}
#endif

#ifndef CONFIG_RTL865X_HW_TABLES
void SoftNAT_OP_Mode(int count)
{
	int i;
	rtl_netif_param_t np;
    	rtl_vlan_param_t vp;
    	struct  init_vlan_setting *setting;    
    	unsigned int ret;    
    	int ethno;
    	struct re_private *cp;
	#if defined(CONFIG_RTL_LAYERED_DRIVER)
	rtl865x_netif_t netif;
	unsigned int realTotalVlans = sizeof(vlanSetting)/sizeof(struct init_vlan_setting);
	#endif
	
    	totalVlans = count;
	setting = vlanSetting;
	savOP_MODE_value=count;
	
#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	if (L2_table_disabled) {

		// eth0(LAN) VLAN, eth2 ~ 4 do not change
		cp = reNet[0]->priv;
#ifdef CONFIG_RTK_GUEST_ZONE

#if defined(CONFIG_RTL8196C_KLD)		
		REG32(PVCR0) = (LAN3_VID << 16) | LAN4_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
		
		cp->port = BIT(3);		
		setting[0].portmask = BIT(3);
#else
		REG32(PVCR0) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN3_VID << 16) | LAN2_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN4_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
		
		cp->port = BIT(1);		
		setting[0].portmask = BIT(1);
#endif
		
#endif
	
#ifdef CONFIG_RTK_VLAN_SUPPORT		
#if defined(CONFIG_RTL8196C) && !defined(CONFIG_RTL8198)
		REG32(PVCR0) = (LAN2_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN4_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | WAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;		
		cp->port = BIT(0);		
		setting[0].portmask = BIT(0);
#else
		REG32(PVCR0) = (LAN4_VID << 16) | WAN_VID;
		REG32(PVCR1) = (LAN2_VID << 16) | LAN3_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
		cp->port = BIT(4);		
		setting[0].portmask = BIT(4);
#endif
#endif

#if defined(CONFIG_RTL8196C) && !defined(CONFIG_RTL8198)
		// eth1(WAN) VLAN
		cp = reNet[1]->priv;
		cp->port = BIT(wan_port);
		setting[1].portmask = BIT(wan_port);
#else
		// eth1(WAN) VLAN
		cp = reNet[1]->priv;
		cp->port = BIT(0);
		setting[1].portmask = BIT(0);
#endif		
		totalVlans = ETH_INTF_NUM;
	}
	else
#endif
	
#ifdef CONFIG_RTL8196_RTL8366
	{
		int iport;
		int32 retval = 0;
		rtl8366rb_vlanConfig_t vlancfg_8366;

		/* for lan */
		if(count == 2) // gateway
		{
			memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
			vlancfg_8366.fid = 0;
			vlancfg_8366.mbrmsk = (RTL8366RB_LANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_LANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
			{
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
			}
			/* for wan */
			iport=0;
			retval = 0;

			vlancfg_8366.fid =0;
			vlancfg_8366.mbrmsk = (RTL8366RB_WANPORT&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
			vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
			vlancfg_8366.vid = RTL_WANVLANID;
			retval = rtl8366rb_setVlan(&vlancfg_8366);
			/*need set pvid??*/
			for(iport=0;iport<8;iport++)
			{
				if  ((1<<iport)&vlancfg_8366.mbrmsk)
				{
					retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
				}
			}
		}
		else
		{
			{
                        /* for lan */
                        int iport;
                        rtl8366rb_vlanConfig_t vlancfg_8366;

                        memset(&vlancfg_8366, 0, sizeof(rtl8366rb_vlanConfig_t));
                        vlancfg_8366.fid = 0;
                        vlancfg_8366.mbrmsk = ((RTL8366RB_WANPORT|RTL8366RB_LANPORT)&RTL8366RB_PORTMASK)|RTL8366RB_GMIIPORT;
                        vlancfg_8366.untagmsk = vlancfg_8366.mbrmsk&(~RTL8366RB_GMIIPORT);
                        vlancfg_8366.vid = RTL_LANVLANID;
                        retval = rtl8366rb_setVlan(&vlancfg_8366);
                        /*need set pvid??*/
                        for(iport=0;iport<8;iport++)
                                if  ((1<<iport)&vlancfg_8366.mbrmsk)
                                {
                                        retval = rtl8366rb_setVlanPVID(iport, vlancfg_8366.vid, 0);
                                }
                	}
                {
                        /* for wan clear it */
                        int iport;
                        rtl8366rb_vlanConfig_t vlancfg_8366;

                        vlancfg_8366.fid = 0;
                        vlancfg_8366.mbrmsk = 0;
                        vlancfg_8366.untagmsk = 0;
                        vlancfg_8366.vid = RTL_WANVLANID;
                        retval = rtl8366rb_setVlan(&vlancfg_8366);
                }

		}
		return;
	}
#elif defined(CONFIG_RTL8196B_GW_MP)
	{
 		REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
		cp = reNet[0]->priv;
		setting[0].portmask = ALL_PORTS;
		cp->port = ALL_PORTS;
		cp = reNet[1]->priv;
		setting[1].portmask = 0;
		cp->port = 0;
	}
#else
	{
	if(count == 2) // gateway
	{
		REG32(PVCR0) = cPVCR[wan_port][0];
		REG32(PVCR1) = cPVCR[wan_port][1];
		REG32(PVCR2) = cPVCR[wan_port][2];
		REG32(PVCR3) = cPVCR[wan_port][3];
		// eth0(LAN) VLAN
		cp = reNet[0]->priv;
#if defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_8196CS) || defined(CONFIG_RTL_ETH_PORT4_ONLY)
		cp->port = 0;
		setting[0].portmask = 0;
/*#elif defined(CONFIG_RTL_8197F)
		cp->port = ALL_PORTS & (~BIT(wan_port));
		setting[0].portmask = ALL_PORTS & (~BIT(wan_port));*/ /*even vlan enabled, netif0 will include all 4 lan ports*/
#else	
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if(rtl_vlan_support_enable)
			cp->port = RTL_LANPORT_MASK_1;
		else
#endif
			cp->port = ALL_PORTS & (~BIT(wan_port));
		setting[0].portmask = ALL_PORTS & (~BIT(wan_port));	/*even vlan enabled, netif0 will include all 4 lan ports*/
#endif

#if defined(CONFIG_RTL_8881A) 
#if defined(CONFIG_RTL_8881AM) || (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH))
		if (cmj_board_use_port4 == 1) {
			cp->port = 0;
			setting[0].portmask = 0;
		}
#endif
#endif
		// eth1(WAN) VLAN
		cp = reNet[1]->priv;
		cp->port = BIT(wan_port);
		setting[1].portmask = BIT(wan_port);
	}
	else if(count == 1) // bridge, WISP
	{
		REG32(PVCR0) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR1) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR2) = (LAN_VID << 16) | LAN_VID;
		REG32(PVCR3) = (LAN_VID << 16) | LAN_VID;
		cp = reNet[0]->priv;

#if defined(CONFIG_POCKET_ROUTER_SUPPORT) || defined(CONFIG_RTL_8196CS) || defined(CONFIG_RTL_ETH_PORT4_ONLY)
		setting[0].portmask = PORT_HW_AP_MODE | CPU_PORT;
		cp->port = PORT_HW_AP_MODE | CPU_PORT;
#elif defined(CONFIG_RTL_8197F)
		cp->port = ALL_PORTS;
		setting[0].portmask = ALL_PORTS;
#else		
#ifdef CONFIG_RTL_VLAN_SUPPORT
		if(rtl_vlan_support_enable)
			cp->port = RTL_LANPORT_MASK_1;
		else
			cp->port = ALL_PORTS; //& (~BIT(wan_port));
#else
			cp->port = ALL_PORTS;		
#endif
		setting[0].portmask = ALL_PORTS;
#endif		

#if defined(CONFIG_RTL_8881A)
#if defined(CONFIG_RTL_8881AM) || (defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH))
		if (cmj_board_use_port4 == 1) {
			setting[0].portmask = PORT_HW_AP_MODE | CPU_PORT;
			cp->port = PORT_HW_AP_MODE | CPU_PORT;
		}
#endif
#endif
		cp = reNet[1]->priv;
        #ifdef CONFIG_RTL_VLAN_SUPPORT
        if (!rtl_vlan_support_enable)
        {
            setting[1].portmask = 0;
            cp->port = 0;
        }
	 else
	 {
	  	cp->port = BIT(wan_port);
	 }
        #else
		setting[1].portmask = 0;
		cp->port = 0;
        #endif
        cp->is_wan = 0;
	}
	else
	{
 		return;
	}	
	}
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_HARDWARE_NAT)
	if(flush_hw_table_flag==1){
		swCore_init(-1);
		flush_hw_table_flag = 0;
	}else
#endif
    	swCore_init(-2);
        
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if(rtl8651_getAsicOperationLayer() >2){
		rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
	}

	WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	//Enable TTL-1
	WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.

#endif

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT) 
	if (L2_table_disabled) {
	  	REG32(MSCR) |= EN_IN_ACL;
		EasyACLRule(0, RTL8651_ACL_CPU);
	}
	else {
	  	REG32(MSCR) &= (~EN_IN_ACL);
		EasyACLRule(0, RTL8651_ACL_PERMIT);
	}	
#endif

#if defined(CONFIG_RTL_8197F)
	rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	WRITE_MEM32(PLITIMR,0);
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT) 
		//port based decision
	rtl865xC_setNetDecisionPolicy(NETIF_PORT_BASED);
	WRITE_MEM32(PLITIMR,0);
		
	if(rtl_vlan_support_enable){
	  	REG32(MSCR) |= EN_IN_ACL;
		EasyACLRule(0, RTL8651_ACL_PERMIT);	//egress
		#if defined(CONFIG_RTL_8367R_SUPPORT)
		rtl865x_enableRtl8367ToCpuAcl();
		#endif
		
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
		setACL_fwd_ipv4_mcast(1);
		EasyACLRule(2, RTL8651_ACL_CPU);
		
		setACL_fwd_ipv4_mcast(10);
		EasyACLRule(11, RTL8651_ACL_CPU);
		#else
		EasyACLRule(1, RTL8651_ACL_CPU);
		EasyACLRule(10, RTL8651_ACL_CPU);
		#endif
	} else
#endif
	{
		#if defined(CONFIG_RTL_MLD_SNOOPING)	
		REG32(MSCR) |= EN_IN_ACL;
		EasyACLRule(0, RTL8651_ACL_PERMIT); //for egress acl permit
		#if defined(CONFIG_RTL_DNS_TRAP)
		//dns trap occupy two acl rules
		add_acl_for_dns_trap(1); //for wan

		setACL_trap_ipv6_mcast(3);
	#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		rtl_add_Acl_for_br_subnet_from_wan(4);
		EasyACLRule(5, RTL8651_ACL_PERMIT);
	#else
		EasyACLRule(4, RTL8651_ACL_PERMIT);
	#endif
	
		//dns trap occupy two acl rules
		add_acl_for_dns_trap(10); //for lan

		setACL_fwd_ipv6_mcast(12);			//for ping6(DA = 33-33-ff-xx-xx-xx)	
		setACL_trap_ipv6_mcast(13); 		// LAN for tarp ipv6 multicast (DA=33-33-xx-xx-xx-xx)
		EasyACLRule(14, RTL8651_ACL_PERMIT);
		
		#else
		setACL_trap_ipv6_mcast(1);
		
	#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		rtl_add_Acl_for_br_subnet_from_wan(2);
		EasyACLRule(3, RTL8651_ACL_PERMIT);
	#else
		EasyACLRule(2, RTL8651_ACL_PERMIT);
	#endif
		
		setACL_fwd_ipv6_mcast(10);			//for ping6(DA = 33-33-ff-xx-xx-xx)	
		setACL_trap_ipv6_mcast(11); 		// LAN for tarp ipv6 multicast (DA=33-33-xx-xx-xx-xx)
		EasyACLRule(12, RTL8651_ACL_PERMIT);
		#endif
		
		#else
	  	REG32(MSCR) |= EN_IN_ACL;
		EasyACLRule(0, RTL8651_ACL_PERMIT);//for egress acl
		#if defined(CONFIG_RTL_DNS_TRAP)	
		//dns trap occupy two acl rules
		add_acl_for_dns_trap(1); //for wan
	
	#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		rtl_add_Acl_for_br_subnet_from_wan(3);
		EasyACLRule(4, RTL8651_ACL_PERMIT);
	#else
		EasyACLRule(3, RTL8651_ACL_PERMIT);
	#endif
	
		//dns trap occupy two acl rules
		add_acl_for_dns_trap(10);//for lan
		EasyACLRule(12, RTL8651_ACL_PERMIT);
		#else
	
	#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		rtl_add_Acl_for_br_subnet_from_wan(1);
		EasyACLRule(2, RTL8651_ACL_PERMIT);
	#else
		EasyACLRule(1, RTL8651_ACL_PERMIT);
	#endif
	
		EasyACLRule(10, RTL8651_ACL_PERMIT);
		#endif
		
		#endif
		#if defined(CONFIG_RTL_8367R_SUPPORT)&&defined(CONFIG_RTL_VLAN_SUPPORT)
		rtl865x_disableRtl8367ToCpuAcl();
		#endif

		#if defined(CONFIG_RTL_DNS_TRAP) && defined(CONFIG_RTL_8367R_SUPPORT) 
		rtl_8367_add_acl_for_dns(0);
		#endif

	}	

#if defined(CONFIG_RTL_LAYERED_DRIVER)
    	for (ethno = 0; ethno < realTotalVlans; ethno++)
#else
	for (ethno = 0; ethno < totalVlans; ethno++)
#endif
    	{            	
		// Create NetIF 
#if defined(CONFIG_RTL_LAYERED_DRIVER)
		if(ethno>=totalVlans && (setting[ethno].is_slave == 0))
			continue;
		
		memset(&netif, 0, sizeof(rtl865x_netif_t));
		if(setting[ethno].is_slave==0){
			netif.type = IF_ETHER;
			if(setting[ethno].is_wan){
				strcpy(netif.name, RTL_DRV_WAN0_NETIF_NAME);
				netif.mtu = wanNetifMtu;
			}
			else{
				strcpy(netif.name, RTL_DRV_LAN_NETIF_NAME);
				netif.mtu = 1500;
			}
		}
		else{
			netif.type = IF_PPPOE;
			strcpy(netif.name, RTL_DRV_PPP_NETIF_NAME);
			netif.mtu = wanNetifMtu;
		}
		
		memcpy(netif.macAddr.octet, &setting[ethno].mac[0], ETHER_ADDR_LEN);
		netif.vid = setting[ethno].vid;
		netif.is_wan = setting[ethno].is_wan;
		netif.is_slave = setting[ethno].is_slave;

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST) ||defined(CONFIG_RTL_HARDWARE_NAT)
		netif.enableRoute=1;
		#else
		netif.enableRoute=0;
		#endif

		
		#if defined(CONFIG_RTL_8197F)
		netif.mtuV6 = netif.mtu;	
		#if defined(CONFIG_RTL_MLD_SNOOPING) && defined(CONFIG_RTL_HARDWARE_MULTICAST)
		netif.enableRouteV6 =1;   
		#endif
		#endif

		if(netif.type == IF_ETHER){
			if (setting[ethno].is_wan) { // WAN (eth1)
				#if defined(CONFIG_RTL_VLAN_SUPPORT)
				if(rtl_vlan_support_enable){
					netif.inAclStart = 1;
					#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
					netif.inAclEnd = 2;
					#else
					netif.inAclEnd = 1;
					#endif
				} else
				#endif
				{
					netif.inAclStart = 1;
					#if defined(CONFIG_RTL_MLD_SNOOPING)
					netif.inAclEnd = 2;
					#else
			   		netif.inAclEnd = 1;
					#endif
					#if defined(CONFIG_RTL_DNS_TRAP)
					netif.inAclEnd += 2;
					#endif
					
					#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
					netif.inAclEnd += 1;
					#endif
				}
			}else { // LAN (eth0) 
				#if defined(CONFIG_RTL_VLAN_SUPPORT)
				if(rtl_vlan_support_enable){
					netif.inAclStart = 10;
					#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
					netif.inAclEnd = 11;
					#else
					netif.inAclEnd = 10;
					#endif
				} else
				#endif
				{
			   		netif.inAclStart = 10;
					#if defined(CONFIG_RTL_MLD_SNOOPING)
			   		netif.inAclEnd = 12;
					#else
					netif.inAclEnd = 10;
					#endif
					#if defined(CONFIG_RTL_DNS_TRAP)
					netif.inAclEnd += 2;
					#endif
				}
			}
		}
        
		rtl865x_addNetif(&netif);
		if(netif.is_slave == 1)
			rtl865x_attachMasterNetif(netif.name, RTL_DRV_WAN0_NETIF_NAME);
#else	
		bzero((void *) &np, sizeof(rtl_netif_param_t));
		np.vid = setting[ethno].vid;
		np.valid = 1;
		memcpy(&np.gMac, &setting[ethno].mac[0], 6);
		np.macAddrNumber = 1;

		if(setting[ethno].vid==WAN_VID)
			np.mtu = wanNetifMtu;
		else
			np.mtu = 1500;
		
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		np.enableRoute=1;
#else
		np.enableRoute=0;
#endif
		if (ethno == 1) { // WAN (eth1)
			#if defined(CONFIG_RTL_VLAN_SUPPORT)
			if (rtl_vlan_support_enable)
			{
				np.inAclStart = 1;
				np.inAclEnd = 1;
			} else
			#endif
			{
				np.inAclStart = 1;
				#if defined CONFIG_RTL_MLD_SNOOPING
				np.inAclEnd = 2;
				#else
		   		np.inAclEnd = 1;
				#endif
				#if defined(CONFIG_RTL_DNS_TRAP)
				netif.inAclEnd += 2;
				
				#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
				netif.inAclEnd += 1;
				#endif
				#endif
			}
		} else { // LAN (eth0)
			#if defined(CONFIG_RTL_VLAN_SUPPORT)
			if(rtl_vlan_support_enable)
			{
				np.inAclStart = 10;
				#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
				np.inAclEnd = 11;
				#else
				np.inAclEnd = 10;
				#endif
			} else
			#endif
			{
		   		np.inAclStart = 10;
				#if defined(CONFIG_RTL_MLD_SNOOPING)
		   		np.inAclEnd = 12;
				#else
				np.inAclEnd = 10;
				#endif
				#if defined(CONFIG_RTL_DNS_TRAP)
				netif.inAclEnd += 2;
				#endif
			}
		}

		ret = swCore_netifCreate(ethno, &np);
		if ( ret != 0 )
			printk("865x-nic: swCore_netifCreate() failed:%d\n", ret );
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT) ||defined(CONFIG_RTL_8197F)
		#if defined(CONFIG_RTL_LAYERED_DRIVER)
		if(ethno<totalVlans)
		#endif
		{
			for(i=0; i<RTL8651_PORT_NUMBER+3; i++)
			{
				if((1<<i)&setting[ethno].portmask){
					rtl8651_setPortToNetif(i, ethno);
				}
			}
		}
#endif
		// Create VLAN 
#if defined(CONFIG_RTL_LAYERED_DRIVER)
		if(ethno < totalVlans)
#endif
		{
			bzero((void *) &vp, sizeof(rtl_vlan_param_t));
			vp.egressUntag = setting[ethno].portmask;
			vp.memberPort = setting[ethno].portmask;
			vp.fid = ethno;

		ret = swCore_vlanCreate(setting[ethno].vid, &vp);  //P1-P4

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
	if (L2_table_disabled) 
		REG32(SWTCR0) = REG32(SWTCR0)  | EnUkVIDtoCPU;
	else
		REG32(SWTCR0) = REG32(SWTCR0)  & (~EnUkVIDtoCPU);
#endif

			#if defined(CONFIG_RTL_VLAN_SUPPORT)
			if (rtl_vlan_support_enable) 
				REG32(SWTCR0) = REG32(SWTCR0)  | EnUkVIDtoCPU;
			else
				REG32(SWTCR0) = REG32(SWTCR0)  & (~EnUkVIDtoCPU);
			#endif
		}

		if ( ret != 0 )
			printk("eth865x: SoftNAT_OP_Mode() failed:%d\n", ret );  
    	}

#if defined(CONFIG_RTL_LAYERED_DRIVER)
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_HARDWARE_NAT)
    /* if defined  CONFIG_RTL_VLAN_SUPPORT and  CONFIG_RTL_HARDWARE_NAT 
        * swCore_init(-1) del netif table failed due to refCnt >1,==>this can lead to call
        * function rtl865x_addNetif failed and ingress acl rule incorrect.so,set netif acl agian.
       */
    if(rtl_vlan_support_enable)
    {
	#if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 11,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 3,0,0);
        #else
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 10,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 2,0,0);
        #endif

	#else
		#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 11,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 2,0,0);
        #else
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 10,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 1,0,0);
        #endif
	#endif
    }
    else
    {
    #if defined(RTL_TRAP_BR_SUBNET_PKT_FROM_WAN_TO_CPU)
		#if defined(CONFIG_RTL_MLD_SNOOPING)
		#if defined(CONFIG_RTL_DNS_TRAP)
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 14,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 5,0,0);
		#else
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 12,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 3,0,0);
		#endif
        #else
		#if defined(CONFIG_RTL_DNS_TRAP)
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 12,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 4,0,0);
		#else
        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 10,0,0);
        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 2,0,0);
		#endif
        #endif
	#else
	        #if defined(CONFIG_RTL_MLD_SNOOPING)
			#if defined(CONFIG_RTL_DNS_TRAP)
	        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 14,0,0);
	        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 4,0,0);
			#else
	        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 12,0,0);
	        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 2,0,0);
			#endif
	        #else
			#if defined(CONFIG_RTL_DNS_TRAP)
	        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 12,0,0);
	        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 3,0,0);
			#else
	        _rtl865x_setACLToNetif(RTL_DRV_LAN_NETIF_NAME, 10, 10,0,0);
	        _rtl865x_setACLToNetif(RTL_DRV_WAN0_NETIF_NAME, 1, 1,0,0);
			#endif
	        #endif
	#endif
    }
#endif
#endif
	
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	if(old_passThru_flag&IP6_PASSTHRU_MASK
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		||old_passThru_flag&PPPOE_PASSTHRU_MASK
		#endif
		)
	{
		bzero((void *) &vp, sizeof(rtl_vlan_param_t));
		vp.egressUntag = ALL_PORTS;
		vp.memberPort = ALL_PORTS;
		vp.fid = 0;
		ret = swCore_vlanCreate(PASSTHRU_VLAN_ID, &vp);
		if ( ret != 0 )
			printk("eth865x: add passthough vlan entry failed:%d\n", ret ); 
	}
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	if(rtl8651_getAsicOperationLayer() >2){
		if(count == 2)	//gateway mode
			rtl865x_addRoute(0,0,0,RTL_DRV_WAN0_NETIF_NAME,0);
		else if(count == 1)	//bridge or wisp mode
			rtl865x_addRoute(0,0,0,RTL_DRV_LAN_NETIF_NAME,0);
	}

	WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	//Enable TTL-1
	WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.
#endif

	#if defined(CONFIG_RTL_8367R_SUPPORT)
	RTL8367R_vlan_set(count);
	#endif

#if 0//defined(CONFIG_RTL_8197F)	//jwj:to-do, now just disable ingress acl.
	REG32(MSCR) &= ~EN_IN_ACL;
#endif

#if defined(CONFIG_RTL_8197F)
	rtl819x_setSwEthPvid();
#if defined(CONFIG_RTL_HARDWARE_MULTICAST)&& defined(CONFIG_RTL_MLD_SNOOPING)
	rtl_reinitMulticastv6();
#endif
#endif
#if defined(CONFIG_RTL_HARDWARE_MULTICAST) 
	rtl865x_setMulticastExternalPortMask(0);
#endif

}
#endif

#ifdef __KERNEL__
unsigned int get_portlist_by_iface(char *iface_name)
{
	int i;
	struct re_private *cp;
	struct net_device *dev = __dev_get_by_name(iface_name);

	if (dev) {
		for (i=0; i<ETH_INTF_NUM; i++) {
			if (reNet[i] != NULL) {
				cp = reNet[i]->priv;	
				if (cp->dev == dev) {
					return ((cp->port & ALL_PORT_MASK));
				}	
			}
		}
	}	
	return 0;
}
#endif

#if defined(__KERNEL__) && !defined(CONFIG_RTL_KERNEL_MIPS16_DRVETH)
void __restore_flags__(unsigned long *x)
{	
	__restore_flags(*x);
}

void __save_and_cli__(unsigned long *x)
{	
	unsigned long flags;		
	__save_and_cli(flags);	
	*x = flags;			
}
#endif

#ifdef __KERNEL__
module_init(rtl865x_init);
module_exit(rtl865x_exit);
#endif

#ifdef __ECOS
void dump_eth_info(void)
{
	diag_printf("opmode=%d\n", savOP_MODE_value);
	diag_printf("wan_port=%d\n", wan_port);
	diag_printf("PVCR0=0x%08x\n", REG32(0xBB804A08));
	diag_printf("PVCR1=0x%08x\n", REG32(0xBB804A0C));
	diag_printf("PVCR2=0x%08x\n", REG32(0xBB804A10));
	diag_printf("PVCR3=0x%08x\n", REG32(0xBB804A14));
	diag_printf("PVCR4=0x%08x\n", REG32(0xBB804A18));
}

void write_proc_wan_port(int portnum)
{
	int opmode = savOP_MODE_value;

	wan_port = portnum; //0~5
	SoftNAT_OP_Mode(opmode);
}

void eth_cmd_dispatch(int argc, char *argv[])
{
	extern void Setting_RTL8196C_PHY(void);
	extern int Setting_RTL8196D_PHY(void);
	extern int Setting_RTL8196E_PHY(void);
	extern int Setting_RTL8881A_PHY(void);
	extern int Setting_RTL8197F_PHY(void);
	extern void dump_rx_desc_own_bit(void);
	extern void dump_tx_desc_own_bit(void);
	extern int rtk_atoi(char *s);

	if (strcmp(argv[0], "info")==0) {
		dump_eth_info();
	}
	else if (strcmp(argv[0], "swreset")==0) {
		FullAndSemiReset();
	}
	else if (strcmp(argv[0], "phyreset")==0) {
#ifdef CONFIG_RTL8196C_REVISION_B
		if (REG32(REVR) == RTL8196C_REVISION_B)
			Setting_RTL8196C_PHY();
#elif defined(CONFIG_RTL_8881A)
		Setting_RTL8881A_PHY();
#elif defined(CONFIG_RTL_819XD)
		Setting_RTL8196D_PHY();
#elif defined(CONFIG_RTL_8196E)
		Setting_RTL8196E_PHY();
#elif defined(CONFIG_RTL_8197F)
		Setting_RTL8197F_PHY();
#endif
	}
	else if (strcmp(argv[0], "desc")==0) {
		dump_rx_desc_own_bit();
		dump_tx_desc_own_bit();
	}
	/*else if (strcmp(argv[0], "mcat")==0) {
		dump_multicast_table();
	}
	else if (strcmp(argv[0], "asiccounter")==0) {
		int clear = 0;
		if(argc > 1)
			clear = 1;
		if (clear)
			rtl8651_clearAsicCounter();
		else
			rtl865xC_dumpAsicCounter();
	}*/
	else if (strcmp(argv[0], "stats")==0) {
		int clear = 0;
		if(argc > 1)
			clear = 1;
		rtl865x_stats(clear);
	}
	else if (strcmp(argv[0], "opmode")==0) {
		int opmode=0; // 1:bridge/WISP, 2:gateway
		//diag_printf("argc=%d\n", argc);
		if (argc > 1) {
			if (strcmp(argv[1], "1")==0)
				opmode = 1;
			else if (strcmp(argv[1], "2")==0)
				opmode = 2;
			
			if ((1==opmode) || (2==opmode)){
				SoftNAT_OP_Mode(opmode);
				return;
			}
		}
		diag_printf("opmode=%d, ", savOP_MODE_value);
		if (savOP_MODE_value == 1)
			diag_printf("br\n");
		else
			diag_printf("gw\n");
	}
	else if (strcmp(argv[0], "l2")==0) {
		extern void rtl865xC_dump_l2(void);
		rtl865xC_dump_l2();
	}
	else if (strcmp(argv[0], "vlan")==0) {
		extern void rtl865xC_dump_vlan(void);
		rtl865xC_dump_vlan();
#if defined(CONFIG_RTL_8367R_SUPPORT)
		extern int rtl_8367r_vlan_read(void);
		 rtl_8367r_vlan_read();
#endif
	}
	else if (strcmp(argv[0], "netif")==0) {
		extern void rtl865xC_dump_netif(void);
		rtl865xC_dump_netif();
	}
	else if (strcmp(argv[0], "wan_port")==0) {
		//diag_printf("argc=%d\n", argc);
		if (argc > 1) {
			int portnum=rtk_atoi(argv[1]);
			if ((portnum>=0) && (portnum<6)) {
				write_proc_wan_port(portnum);
			}
			else {
				diag_printf("inport port num is invalid\n");
				return;
			}
		}
		diag_printf("wan_port=%d\n", wan_port);
	}
	#if 1//defined(CONFIG_RTL_VLAN_SUPPORT)
	else if(strcmp(argv[0], "acl")==0){
		extern void showAclRule(void);
		showAclRule();
	}
	#endif
	#if 1//defined PROC_DEBUG
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	else if (strcmp(argv[0], "mcat")==0) {
		dump_multicast_table();
	}
	#endif
	#if 1//defined ASICCOUNTER_DEBUG
	else if (strcmp(argv[0], "asicCounter")==0) {
		extern int32 rtl865xC_dumpAsicDiagCounter(void);
		extern int32 rtl8651_clearAsicCounter(void) ;
		extern int32 rtl865xC_dumpPortAsicCounter(uint32 portNum);
		uint32 port=0xFF;
		int clear = 0;
		if(argc > 1){
			if(strcmp(argv[1], "clear")==0)
				clear = 1;
			else if(strcmp(argv[1], "port")==0){
				if((strcmp(argv[2], "all")==0))
					rtl865xC_dumpAsicDiagCounter();
				else
				{
					port=strtol(argv[2], NULL, 0);
					rtl865xC_dumpPortAsicCounter(port);
				}
			} 
			#ifdef CONFIG_RTL_8367R_SUPPORT
			else if (strcmp(argv[1], "8367")==0) {
				extern void rtl_get8367rPortStats(void);
				rtl_get8367rPortStats();
			}
			#endif
		}
		else
			rtl865xC_dumpAsicDiagCounter();
		
		if (clear)
			rtl8651_clearAsicCounter();
		
	}
	#endif
	#if 1//defined DIAGNOSTIC_DEBUG
	//diagnostic
	else if(strcmp(argv[0], "diagnostic")==0){
		extern void diagnostic_show(void);
		diagnostic_show();
	}
	#endif
	#if 1//defined PORTSTATUS_DEBUG
	//port_status
	else if(strcmp(argv[0], "portStatus")==0){
		extern int32 port_status_read( void );
		extern int32 port_status_write( uint32	port_mask, int 	port_type );
		uint32	port_mask =0;
		int 	port_type =0;
		if(argc > 1){
			if (strcmp(argv[1], "read")==0){
				diag_printf("read portstatus.\n");
				port_status_read();
			}	
			else if (strcmp(argv[1], "write")==0){
				diag_printf("not support yet!\n");	
				#if	0
				if(argc > 3)
				{
					port_mask=strtol(argv[2], NULL, 0);
					port_type=strtol(argv[3], NULL, 0);
					//diag_printf("write portmask(%x) port_type(%x).\n",port_mask,port_type);
					port_status_write(port_mask, port_type );

				}
				else{
					diag_printf("parameter error!\n");
				}
				#endif
			}	
			else{
					diag_printf("parameter error!\n");
			}
				
		}
		else	
			port_status_read();
	}
	#endif
	#if 1//defined PVID_DEBUG
	//pvid
	else if(strcmp(argv[0], "pvid")==0){
		extern int32 pvid_read( void );
		extern int32 pvid_write(uint32 port, uint32 pvid);
		
		uint32	port =0;
		int 	pvid =0;
		if(argc > 1){
			if (strcmp(argv[1], "read")==0)
				pvid_read();
			else if (strcmp(argv[1], "write")==0){
				if(argc > 3)
				{
					port=strtol(argv[2], NULL, 0);
					pvid=strtol(argv[3], NULL, 0);
					diag_printf("write port(%d) pvid(%d) .\n",port,pvid);
					pvid_write(port, pvid );

				}
				else{
					diag_printf("parameter error!\n");
				}
			}	
			else{
					diag_printf("parameter error!\n");
			}
				
		}
		else	
			pvid_read();
	}	
	#endif
	#if defined DBG_PHY_REG
	//phy
	else if(strcmp(argv[0], "phy")==0)
	{
		extern void phy_read_all(void);
		extern void phy_op(uint32 op, uint32 id, uint32 page, uint32 reg, uint32 *val);
		
		
		uint32 read=1;
		uint32 id, page, reg,value;
		#ifdef CONFIG_RTL_8367R_SUPPORT
		int ret;
		#endif
		
		if((strcmp(argv[1], "read")==0)|| (strcmp(argv[1], "write")==0))	
		{
			if (strcmp(argv[1], "write")==0) 
				read = 0;
				
			if(strcmp(argv[2], "all")==0){
				if (read ) {
					diag_printf("phy read all.\n");
					phy_read_all();
					return ;
				}
			}	
			
			// phy id (port id)
			else {
				
				id=strtol(argv[2], NULL, 0);
				
				page=strtol(argv[3], NULL, 0);
				// page
				
				reg=strtol(argv[4], NULL, 0);
				// register number
				
				if (read == 0) {
					
					value=strtol(argv[5], NULL, 0);
					// register value
					
				}
				//diag_printf("read(%d) id(%x) page(%x) reg(%x) value(%x) .\n",read,id,page,reg,value);
				phy_op(read, id, page, reg, &value);
			}
		}
#ifdef CONFIG_RTL_8367R_SUPPORT
		else if (!memcmp(argv[1], "8367read", 8))
		{
			reg=strtol(argv[2], NULL, 16);
			ret = rtl8367b_getAsicReg(reg, &value);

			if (ret==0) {
				diag_printf("rtl8367b_getAsicReg: reg= %x, data= %x\n", reg, value);
			} else {
				diag_printf("get fail %d\n", ret);
			}
		}
		else if (!memcmp(argv[1], "8367write", 9))
		{
			reg=strtol(argv[2], NULL, 16);
			value=strtol(argv[3], NULL, 16);
			ret = rtl8367b_setAsicReg(reg, value);

			if (ret==0) {
				diag_printf("rtl8367b_setAsicReg: reg= %x, data= %x\n", reg, value);
			} else {
				diag_printf("set fail %d\n", ret);
			}
		}
		else if (!memcmp(argv[1], "8367phyr", 8))
		{
			extern int rtl8367b_getAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 *pRegData );

			id=strtol(argv[2], NULL, 0);
			reg=strtol(argv[3], NULL, 0);

			ret=rtl8367b_getAsicPHYReg(id, reg, &value);
			if(ret==SUCCESS){
				diag_printf("read 8367R/RB phyId(%d), regId(%d), regData:0x%x\n", id, reg, value);
			}
			else{
				diag_printf("error input!\n");
			}		
		}
		else if (!memcmp(argv[1], "8367phyw", 8))
		{
			extern int rtl8367b_setAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 phyData );
		
			id=strtol(argv[2], NULL, 0);
			reg=strtol(argv[3], NULL, 0);
			value=strtol(argv[4], NULL, 16);

			ret=rtl8367b_setAsicPHYReg(id, reg, value);
			if(ret==SUCCESS){
				rtlglue_printf("Write 8367R/RB phyId(%d), regId(%d), regData:0x%x\n", id, reg, value);
			}else{
				rtlglue_printf("error input!\n");
			}
		}		
#endif
		else
			diag_printf("parameter error!\n");
	
	}
	#endif
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	//igmp
	else if (strcmp(argv[0], "igmp")==0){
		extern void igmp_cmd_op(int argc, char *argv[]);
		
		igmp_cmd_op(argc, (char **)argv);

	}	
	#endif
	else if (strcmp(argv[0], "l3")==0){
		extern void l3_show(void);
		l3_show();
	}	
	#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	else if(strcmp(argv[0],"passthru")==0)
	{
		uint32 mask_value=0;
		if (argc >= 2)
		{
			if (strcmp(argv[1], "read")==0){
				passthru_show();
				
			}	
			else if ((strcmp(argv[1], "write")==0))
			{
				if(argc > 2){
					mask_value=strtol(argv[2], NULL, 0);
					passThru_flag[0]=mask_value;
					rtl8651_customPassthru_infoSetting();
				}	
				
			}
		}	
		else{

			passthru_show();
		}
	}
	#endif
	#if 0
	//privSkbInfo	
	else if(strcmp(argv[0], "privSkbInfo")==0){
		extern void txRing_show(void);
		txRing_show();();
	}
	//mmd
	else if(strcmp(argv[0], "mmd")==0){
		extern void txRing_show(void);
		txRing_show();();
	}	
	
	//maccr
	else if(strcmp(argv[0], "maccr")==0){
		extern void txRing_show(void);
		txRing_show();();
	}		
	//fc_threshold
	else if(strcmp(argv[0], "fc_threshold")==0){
		extern void txRing_show(void);
		txRing_show();();
	}		
	
	#endif
	#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	else if (strcmp(argv[0], "ip")==0){
		extern void rtl865x_asicIpShow(void);
		rtl865x_asicIpShow();
	}	
	else if (strcmp(argv[0], "arp")==0){
		extern void rtl865x_asicArpShow(void);
		rtl865x_asicArpShow();
	}
#if defined(CONFIG_RTL_HARDWARE_NAT)	
	else if (strcmp(argv[0], "add_arp")==0){
		extern int rtl865x_addArp(unsigned long ip, unsigned char * mac);
		unsigned long temp_ip = 0;
		inet_aton(argv[1], &temp_ip);	
		
		unsigned char mac_addr[ETHER_ADDR_LEN];
		printf ("%s %d  ip=%s= mac=%s=\n", __FUNCTION__, __LINE__, argv[1], argv[2]);
		ether_atoe(argv[2], mac_addr);
		
		rtl865x_addArp(ntohl(temp_ip) , mac_addr);
	}
	else if (strcmp(argv[0], "dump_tbl")==0)
	{		
	printf ("%s %d \n", __FUNCTION__, __LINE__);
		extern void dump_netifTbl(void);
	       dump_netifTbl();
	}
#endif	
	else if (strcmp(argv[0], "nexthop")==0){
		extern void rtl865x_asicNxtHopShow(void);
		rtl865x_asicNxtHopShow();
	}	
	else if (strcmp(argv[0], "ppp")==0){
		extern void rtl865x_asicPppShow(void);
		rtl865x_asicPppShow();
	}
	else if (strcmp(argv[0], "event")==0){
		extern void rtl865x_asicEventShow(void);
		rtl865x_asicEventShow();
	}
	#endif
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	else if (strcmp(argv[0], "napt")==0){
		extern void rtl865x_asicNaptShow(void);
		rtl865x_asicNaptShow();
	}
	#endif
	#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
	else if (strcmp(argv[0], "fullstats")==0){
		if (argc >= 2)
		{
			if (strcmp(argv[1], "read")==0){
				goto show_stats;
			}	
			else if ((strcmp(argv[1], "clear")==0))
			{
				rx_noBuffer_cnt=0;
				rx_noBuffer_cnt1=0;
				tx_ringFull_cnt=0;
				
			}
		}	
		else{
		show_stats:
			diag_printf( "  Debug Statistics Info:\n");
			diag_printf( "    rx_noBuffer_cnt:	%d\n",rx_noBuffer_cnt);
			diag_printf( "    rx_noBuffer_cnt1:	%d\n",rx_noBuffer_cnt1);
			diag_printf( "    tx_ringFull_cnt:	%d\n",tx_ringFull_cnt);
		}
	}
	#endif
	#endif
    #if defined(CONFIG_RTL_PHY_POWER_CTRL)    
	else if(strcmp(argv[0], "phypower")==0)
	{
	    uint32 portnum = 0;
        extern int rtl_setPhyPowerOff(unsigned int port);        
        extern int rtl_setPhyPowerOn(unsigned int port);       
        extern void rtl_phyPowerCtrlShow(void);
	    if (argc == 4)
        {
            if (strcmp(argv[1], "port")==0)
            {
                portnum = strtol(argv[2], NULL, 0);
                if (strcmp(argv[3], "on") == 0)
                {
                    rtl_setPhyPowerOn(portnum);
                }
                else if (strcmp(argv[3], "off") == 0)
                {
                    rtl_setPhyPowerOff(portnum);
                }
            }
            else
            {
                diag_printf("parameter error!\n");
            }
        }
        else if(argc == 2)
        {
            if (strcmp(argv[1], "show")==0)
            {              
                rtl_phyPowerCtrlShow();
            }
        }
        else
        {
            diag_printf("parameter error!\n");
            diag_printf("format:eth phypower port portnum on/off\n");           
            diag_printf("       eth phypower show\n");
        }
    }
    #endif
}
#endif

#if defined(CONFIG_RTL_819X)
int rtl_isWanDevDecideByName(unsigned char *name)
{
	int i;
	
	for(i=0; i<ETH_INTF_NUM; i++)
	{
		if(reNet[i] && (!memcmp(reNet[i]->name, name, strlen(name)))){
			return ((struct re_private *)reNet[i]->priv)->is_wan;
		}
	}

	return 0;
}
#endif

void rtl_setWanNetifMtu(int mtu)
{
	if(mtu<=0||mtu>1500)
		wanNetifMtu = 1500;
	else
		wanNetifMtu = mtu;
}

#ifdef __ECOS
int ecos_send_eth(struct net_device *dev, unsigned char *data, int size)
{
	struct re_private *priv;
	struct sk_buff *pskb;
	
	priv = dev->priv;
	pskb = dev_alloc_skb(size);
	if (pskb == NULL) {
		DEBUG_ERR(" allocate skb failed!\n");
		return -1;
	}
	memcpy(pskb->data, data, size);
	skb_put(pskb, size);

	//pskb->cb[0] = 7;	// use highest priority to xmit
	if (rtl865x_start_xmit(pskb, priv->dev)){
		DEBUG_ERR("tx \n");
		dev_kfree_skb_any(pskb);
	}
	return 0;
}
#endif
#if defined(CONFIG_RTL_8881A)
#if defined(CONFIG_CUTE_MAHJONG) && defined(CONFIG_CMJ_SWITCH)
void rtl_setUnusedPortPowerOff(void)
{
    uint32 statCtrlReg0 = 0, index = 0; 
    //if ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8881AM) 
    {
        /* disable unused port for saving power p0/p2/p3/p4 */
        for (index = 0; index < RTL8651_PHY_NUMBER; index++) 
        {
            //if (index != 1) 
            //if (((1<<index)&(~RTL_WANPORT_MASK)) != 0)
            if (index != wan_port) 
            {
                /* read current PHY reg 0 value */
                rtl8651_getAsicEthernetPHYReg( index, 0, &statCtrlReg0 );

                REG32(PCRP0+(index*4)) |= EnForceMode;
                statCtrlReg0 |= POWER_DOWN;

                /* write PHY reg 0 */
                rtl8651_setAsicEthernetPHYReg( index, 0, statCtrlReg0 );
            }
        }
    }
    return;
}
#endif
#endif

#if defined (CONFIG_RTL_PHY_POWER_CTRL)
int rtl_setPhyPowerOff(unsigned int port)
{
	unsigned int offset = port * 4;
	unsigned int pcr = 0;
	unsigned int regData;
	unsigned int macLinkStatus;
	unsigned int phyLinkStatus;
	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}
    
    #ifdef CONFIG_RTL_8367R_SUPPORT
    extern int rtl_8367r_phypowerOnOff(unsigned int port, int onoff);
    rtl_8367r_phypowerOnOff(port, 0);
    #else
#if defined(CONFIG_RTL_8197F)
	if (port == 0)
		port = 8;		//RTL8198C_PORT0_PHY_ID
#endif
	/*must set mac force link down first*/
	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;
	pcr &= ~ForceLink;
	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData |(1<<11);
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	/*confirm shutdown phy power successfully*/
	regData = READ_MEM32(PSRP0+offset);
	macLinkStatus = regData & PortStatusLinkUp;

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	phyLinkStatus=!(regData & (1<<11));
	while((macLinkStatus) || (phyLinkStatus) )
	{
		//printk("port is %d,macLinkStatus is %d,phyLinkStatus is %d\n",port,macLinkStatus,phyLinkStatus);
		pcr=READ_MEM32( PCRP0 + offset );
		pcr |= EnForceMode;
		pcr &= ~ForceLink;
		WRITE_MEM32( PCRP0 + offset, pcr );
		TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		regData=regData |(1<<11);
		rtl8651_setAsicEthernetPHYReg(port, 0, regData);

		regData = READ_MEM32(PSRP0+offset);
		macLinkStatus = regData & PortStatusLinkUp;

		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
		phyLinkStatus=!(regData & (1<<11));
	}
    #endif

	return 0;

}

int rtl_setPhyPowerOn(unsigned int port)
{
	unsigned int  offset = port * 4;
	unsigned int pcr = 0;

	unsigned int regData;

	if(port >=RTL8651_PHY_NUMBER)
	{
		return -1;
	}
    #ifdef CONFIG_RTL_8367R_SUPPORT
    extern int rtl_8367r_phypowerOnOff(unsigned int port, int onoff);
    rtl_8367r_phypowerOnOff(port, 1);
    #else
#if defined(CONFIG_RTL_8197F)
	if (port == 0)
		port = 8;		//RTL8198C_PORT0_PHY_ID
#endif

	pcr=READ_MEM32( PCRP0 + offset );
	pcr |= EnForceMode;

	WRITE_MEM32( PCRP0 + offset, pcr );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);

	rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
	regData=regData &(~(1<<11));
	rtl8651_setAsicEthernetPHYReg(port, 0, regData);

	pcr &=~EnForceMode;
	WRITE_MEM32( PCRP0 + offset, pcr);
	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
    #endif
    
	return 0;

}


void rtl_phyPowerCtrlShow(void)
{
	int port;
	unsigned int regData = 0;
	for(port=0;port<RTL8651_PHY_NUMBER;port++)
	{   
	    regData = 0;
        #ifdef CONFIG_RTL_8367R_SUPPORT
        extern int rtl_8367r_port_phyreg_get(unsigned int port, unsigned int *pData);
        rtl_8367r_port_phyreg_get(port, &regData);
        #else
		rtl8651_getAsicEthernetPHYReg(port, 0, &regData);
        #endif
		if(regData & (1<<11))
		{
		    diag_printf("port[%d] phy is power off\n", port);
		}
		else
		{		
            diag_printf("port[%d] phy is power on\n", port);
		}
	}

	return ;
}
#endif
#if defined(CONFIG_RTL_8881A_CUTE_MAHJONG_ESD)
static void rtl_bsp_machine_restart(char *command)
{
    static void (*back_to_prom)(void) = (void (*)(void)) 0xbfc00000;
	
    REG32(GIMR)=0;
	
    //local_irq_disable();    
    cyg_interrupt_disable();
#ifdef CONFIG_NET    
    shutdown_netdev();
#endif    
    REG32(BSP_WDTCNR) = 0; //enable watch dog
    while (1) ;
    /* Reboot */
    back_to_prom();
}

void rtl_CuteMahjongEsdTimerHandle(void)
{

    if (_cute_mahjong_esd_counter) {
        //if( (REG32(PCRP1) & EnablePHYIf) == 0) //if check port changed, please modify here.
        if( (REG32(PCRP0 + (wan_port*4)) & EnablePHYIf) == 0) //if check port changed, please modify here.
        {
            if (++_cute_mahjong_esd_reboot_counter >= 20) {
                diag_printf("  ESD reboot...\n");
                //machine_restart(NULL);
                rtl_bsp_machine_restart(NULL);
            }
            //diag_printf("%s %d _cute_mahjong_esd_reboot_counter=%d\n", __FUNCTION__, __LINE__, _cute_mahjong_esd_reboot_counter);
        }
        else {
            _cute_mahjong_esd_reboot_counter = 0;
        }
    }

    return;
}

void rtl_startCuteMahjondEsdCheck(void)
{
    int i = 0;
    
    for (i = 0; i < 5; i++)
    {
        if (((1<<i)&(RTL_WANPORT_MASK))) 
        {
            // default port 1
            _cute_mahjong_esd_counter = 1;      // start counting and check ESD
            _cute_mahjong_esd_reboot_counter = 0;   // reset counter
        }
    }

    return;
}

void rtl_stopCuteMahjondEsdCheck(int index)
{
    
    if (((1<<index)&(RTL_WANPORT_MASK))) //wan port(default port1)
    {
        //diag_printf("%s %d index = %d RTL_WANPORT_MASK=%d\n", __FUNCTION__, __LINE__, index, RTL_WANPORT_MASK);
        _cute_mahjong_esd_counter = 0; //stop counting
    }

    return;
}
#endif

#ifdef CONFIG_RTL_ULINKER
void eth_led_recover(void)
{
	REG32(PIN_MUX_SEL2) = REG32(PIN_MUX_SEL2) & ~(0x00003000);
}
#endif

#if defined(CONFIG_RTL_8197F)
uint32 rtl819x_bond_option(void)
{
        unsigned int type = 0, ret = 0;

        type = REG32(BSP_BOND_OPTION) & 0xf;

        switch(type) {
        case 0x0:       /* 97FB */
                ret = BSP_BOND_97FB;
                break;
        case 0x4:       /* 97FN */
        case 0x5:
        case 0x6:
                 ret = BSP_BOND_97FN;
                break;
        case 0xa:       /* 97FS */
        case 0xb:
        case 0xc:
                ret = BSP_BOND_97FS;
        }

        //diag_printf("[%s][%d]: 97F type %d\n", __FUNCTION__, __LINE__, ret);//BSPLJF-- for "wlan1 mib_rf" messy code
        return ret;
}
#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
void rtl_dev_kfree_skb_any(struct sk_buff *skb)
{
	return dev_kfree_skb_any(skb);
}
#endif
#endif

#if defined(CONFIG_RTL_8197F)
int rtl819x_setSwEthPvid()
{
	uint32 port, pvid;
	
	for(port = 0; port < RTL8651_PORT_NUMBER; port++)
	{
		rtl8651_getAsicPvid(port, &pvid);
		sw_pvid[port] = pvid;
	}
	
	return SUCCESS;
}

int rtl819x_getSwEthPvid(uint32 port, uint32* pvid)
{
	if (port >= RTL8651_PORT_NUMBER)
		return FAILED;

	*pvid = sw_pvid[port];
	return SUCCESS;
}
#endif
#if defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
//call this function in init flow.
void rtl_set_wan_if_name(void) //(int op_mode, int wlan_mode, int wisp_wan_id)
{
	snprintf(wan_if_name, 15, "%s", nvram_safe_get("wan0_ifname"));
    return;
}

unsigned char *rtl_get_wan_if_name(void)
{
	return wan_if_name;
}


/* 
*  dir means call direction, if call in driver rx direction, dir = RTL_RX_DIR, if call in driver tx direction dir = RTL_TX_DIR
*  please pay attention to function return value!!!!
*  eg: if skb freed in this function, function caller need skip the after process.
*/
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
extern void get_apclient_info(char* mbuf_data,struct ip *iph);
extern int work_mode_is_route;
#endif
static inline struct ip* rtl_qos_get_iph(struct sk_buff *skb)
{
	struct ip *iph = NULL;
	uint16 type = 0;
	if (!skb || !skb->data)
		return NULL;


	type = ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1))));
	if (type == ETH_P_IP ||
	    (type == ETH_P_8021Q && ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN)))==ETH_P_IP)||
	    (type == ETH_P_8021Q && ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN)))==ETH_P_PPP_SES)||
	    (type == ETH_P_PPP_SES && ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+8)))==0x0021))
	{

		if (type == ETH_P_8021Q)
		{
			if (ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN)))==ETH_P_IP)
			{
				iph=(struct ip *)(skb->data+(ETH_ALEN<<1) + 2 + VLAN_HLEN);
			}
			else if ((ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN)))==ETH_P_PPP_SES)
			         && (ntohs(*((uint16*)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+8)))==0x0021))
			{
				iph = (struct ip *)(skb->data+(ETH_ALEN<<1) + 2 + VLAN_HLEN +8);
			}
		}
		else if (type == ETH_P_PPP_SES)
		{
			iph=(struct ip *)(skb->data+(ETH_ALEN<<1) + 2 + 8);
		}
		else
		{
			iph=(struct ip *)(skb->data+(ETH_ALEN<<1) + 2);
		}
	}
	return iph;
}

typedef enum{
	PRI_UP_LIMIT = 0,
	PRI_DOWN_LIMIT,
	GUEST_UP_LIMIT,
	GUEST_DOWN_LIMIT,
	NO_LIMIT
}limit_type;

limit_type rtl_qos_get_nettype(int dir,struct ip *iph)
{
	limit_type type = NO_LIMIT;

	if (dir == RTL_RX_DIR)
	{
		if ((iph->ip_dst.s_addr & br0_ip_mask) != (iph->ip_src.s_addr& br0_ip_mask)
		    && (br0_ip_addr & br0_ip_mask) == (iph->ip_src.s_addr & br0_ip_mask)){
			  	type = PRI_UP_LIMIT;
		   	}
		else if((iph->ip_dst.s_addr & guest_ip_mask) != (iph->ip_src.s_addr& guest_ip_mask)
		    && (guest_ip_addr & guest_ip_mask) == (iph->ip_src.s_addr & guest_ip_mask)){
		     		type = GUEST_UP_LIMIT;
			}
	}else if (dir == RTL_TX_DIR)
	{
		//tx to lan device, check down stream qos speed limit.
		if ((iph->ip_dst.s_addr & br0_ip_mask) != (iph->ip_src.s_addr& br0_ip_mask)
		    && (br0_ip_addr & br0_ip_mask) == (iph->ip_dst.s_addr & br0_ip_mask)){
		    		type = PRI_DOWN_LIMIT;
		    }
		else if((iph->ip_dst.s_addr & guest_ip_mask) != (iph->ip_src.s_addr& guest_ip_mask)
		    && (guest_ip_addr & guest_ip_mask) == (iph->ip_dst.s_addr & guest_ip_mask)){
		    		type = GUEST_DOWN_LIMIT;
		    }
	}
	return type;
}
int rtl_qos_tc_limit(int dir,struct ip *iph)
{
	limit_type type = NO_LIMIT;
	type = rtl_qos_get_nettype(dir,iph);

	if(type == PRI_UP_LIMIT || GUEST_UP_LIMIT == type)//访客网络只做下行限速
	{
		if(g_user_rate_control && type == PRI_UP_LIMIT)
		{
			if(up_win_stream_control(NAT_OUTBOUND, iph) == -1)	  //up
			{					//FREE this packet!
				return SUCCESS;
			}
		}
		stream_statistic(iph, NAT_OUTBOUND,type);
	}else if((type == PRI_DOWN_LIMIT || GUEST_DOWN_LIMIT == type)
			&& (iph->ip_p == PROTO_TCP 
			|| iph->ip_p == PROTO_UDP 
			|| iph->ip_p == PROTO_GRE))
	{
		if(g_user_rate_control && down_win_stream_control(NAT_INBOUND, iph,type) == -1)	  //down			{					//FREE this packet!
		{
			return SUCCESS;
		}
		stream_statistic(iph, NAT_INBOUND,type);
	}

	return FAILED;

}

int tenda_bridge_stream_(int dir,struct ip *iph)
{

	if(dir == RTL_RX_DIR&& (iph->ip_p == PROTO_TCP 
			|| iph->ip_p == PROTO_UDP ))
	{
		bridge_stream_statistic(iph, 0);
	}else if((dir == RTL_TX_DIR)
			&& (iph->ip_p == PROTO_TCP 
			|| iph->ip_p == PROTO_UDP))
	{
		bridge_stream_statistic(iph, 1);
	}

	return FAILED;

}

int rtl_qos_rate_limite_check(int dir, unsigned char *dev_name, struct sk_buff *skb)
{
	struct ip *iph = NULL;
	unsigned char *if_name = NULL;


	//add by z10312 改善转发性能20161020
#ifdef __CONFIG_SPEEDTEST_IMPROVE__
	extern int g_speedtest_web_update_tc_timeout;
	extern int g_speedtest_reboot_check;
	//没有tc 限速及非ap 模式, 页面超期没有打开, 不做以下处理
	if (g_user_rate_control == 0 && work_mode_is_route &&
	    g_speedtest_web_update_tc_timeout == 0  &&
	    g_speedtest_reboot_check == 0 )
	{
		return FAILED;
	}
#endif

	if (!dev_name || !skb || !skb->data)
	{
		return FAILED;
	}

	iph = rtl_qos_get_iph(skb);
	if(iph == NULL)
		return FAILED;

#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
	/*lq 此处为截取dhcp request 数据包的位置，这里只处理wlan0口并且是上传的数据包*/
	if(work_mode_is_route==0 && dir == RTL_RX_DIR
	   &&(strcmp(dev_name, "wlan0") == 0 || strcmp(dev_name, "wlan1") == 0 ))
	{
		get_apclient_info(skb->data,iph);
	}
#endif

	if_name = rtl_get_wan_if_name();
	if (if_name &&(strcmp(dev_name, if_name) !=0 ))
	{
		if(work_mode_is_route)
		{
			return rtl_qos_tc_limit(dir,iph);
		}else
		{
			tenda_bridge_stream_(dir,iph);
		}
	}


	return FAILED;
}
#endif
