/*
 * Abstract: Switch core polling mode NIC driver source code.
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 
#ifdef __ECOS
#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#endif

#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "swNic_poll.h"

#ifdef __KERNEL__
#include <linux/skbuff.h>
#include <linux/delay.h>
#endif

#ifdef CONFIG_RTL8196_RTL8366
#include "RTL8366RB_DRIVER/gpio.h"
#include "RTL8366RB_DRIVER/rtl8366rb_apiBasic.h"
#endif
#define delay_ms(x) mdelay(x)
#define delay_us(x) udelay(x)

#define RTL8651_CPU_PORT                0x07
#define _PKTHDR_CACHEABLE			1

#if defined(CONFIG_RTK_VOIP_WAN_VLAN)|| defined(CONFIG_RTK_VOIP_865xC_QOS)

#define DEFAULT_WAN_VLAN_ID 		8
#define DEFAULT_LAN_VLAN_ID	 	9

#define DEFAULT_VOICE_PRIORITY		7
#define DEFAULT_VIDEO_PRIORITY		4
#define DEFAULT_DATA_PRIORITY		0

#endif
//#define SWITCH_PATCH 1
#ifdef CONFIG_RTK_VOIP_WAN_VLAN
unsigned int 	wan_vlan_enable = 0;

unsigned int    wan_vlan_id_proto = DEFAULT_WAN_VLAN_ID;
unsigned int    wan_vlan_id_data  = DEFAULT_WAN_VLAN_ID;
unsigned int    wan_vlan_id_video = DEFAULT_WAN_VLAN_ID;
                
//for priority and CFI of VLAN field
unsigned int 	wan_priority_proto = DEFAULT_VOICE_PRIORITY; 
unsigned int	wan_priority_data  = DEFAULT_DATA_PRIORITY; 
unsigned int	wan_priority_video = DEFAULT_VIDEO_PRIORITY;

//for CFI of vlan field
unsigned int 	wan_cfi_proto = 0;
unsigned int	wan_cfi_data = 0;
unsigned int	wan_cfi_video = 0;
extern void add_WAN_VLAN(unsigned int);
extern void del_WAN_VLAN(unsigned short);
#endif


#if defined(__KERNEL__) && !defined(CONFIG_RTL_KERNEL_MIPS16_DRVETH)
extern void __restore_flags__(unsigned long *x);
extern void __save_and_cli__(unsigned long *x);

#undef restore_flags
#undef save_and_cli
#define restore_flags(x)	__restore_flags__(&x)
#define save_and_cli(x) __save_and_cli__(&x)
#endif


#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
int reused_skb_num=0;
#endif

extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);
extern void tx_done_callback(void *skb);

/* RX Ring */
static uint32*  rxPkthdrRing[RTL865X_SWNIC_RXRING_MAX_PKTDESC];                 /* Point to the starting address of RX pkt Hdr Ring */
static uint32   rxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC];              /* Total pkt count for each Rx descriptor Ring */
//static uint32 rxPkthdrRingIndex[RTL865X_SWNIC_RXRING_MAX_PKTDESC];            /* Current Index for each Rx descriptor Ring */

/* TX Rings, always initial two rings even we only use the ring 0 */
//static uint32*  txPkthdrRing[RTL865X_SWNIC_TXRING_MAX_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8198C) ||defined(CONFIG_RTL_8197F)
#define CONFIG_RTL_ENHANCE_RELIABILITY		1
#endif

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
static uint32 txPkthdrRing_base[RTL865X_SWNIC_TXRING_MAX_PKTDESC];
static uint32 rxPkthdrRing_base[RTL865X_SWNIC_RXRING_MAX_PKTDESC];
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
uint32 rxMbufRing_base;
#endif
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
static uint32*  txPkthdrRing[4];             /* Point to the starting address of TX pkt Hdr Ring */
#else
static uint32*  txPkthdrRing[2];             /* Point to the starting address of TX pkt Hdr Ring */
#endif

#ifdef CONFIG_RTL8196C_REVISION_B
static uint32*  txPkthdrRing_BACKUP[RTL865X_SWNIC_TXRING_MAX_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */
#endif 
static uint32   txPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC];          /* Total pkt count for each Tx descriptor Ring */
//static uint32 txPkthdrRingFreeIndex[RTL865X_SWNIC_TXRING_MAX_PKTDESC];    /* Point to the entry can be set to SEND packet */

#define txPktHdrRingFull(idx)   (((txPkthdrRingFreeIndex[idx] + 1) & (txPkthdrRingMaxIndex[idx])) == (txPkthdrRingDoneIndex[idx]))

/* Mbuf */
uint32* rxMbufRing=NULL;                                                     /* Point to the starting address of MBUF Ring */
uint32  rxMbufRingCnt;                                                  /* Total MBUF count */

#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
uint32  size_of_cluster;
#else
static uint32  size_of_cluster;
#endif

/* descriptor ring tracing pointers */
static int32   currRxPkthdrDescIndex;      /* Rx pkthdr descriptor to be handled by CPU */
static int32   currRxMbufDescIndex;        /* Rx mbuf descriptor to be handled by CPU */
static int32   currTxPkthdrDescIndex;      /* Tx pkthdr descriptor to be handled by CPU */

static int32 txPktDoneDescIndex;

/* debug counters */
static int32   rxPktCounter;
static int32   txPktCounter;

#ifdef DELAY_REFILL_ETH_RX_BUF
static int32   rxDescReadyForHwIndex;
#endif

#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
extern int L2_table_disabled;
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)
extern int rtl_vlan_support_enable;
#endif

#if	defined(CONFIG_RTL_HARDWARE_NAT)
#define PKTHDR_EXTPORTMASK_CPU	1<<3
#define	PKTHDR_EXTPORT_MAGIC		0xA530
#define	PKTHDR_EXTPORT_MAGIC2		0xA531
#define	_RTL865XB_EXTPORTMASKS   7
static uint8 extPortMaskToPortNum[_RTL865XB_EXTPORTMASKS+1] =
{
	5, 6, 7, 5, 8, 5, 5, 5
};
inline int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid);
#endif


#define ARPTAB_SIZ 16

//--------------------------------------------------------------------------
struct arptab_s
    {
    uint32  port_list;
    uint8   arp_mac_addr[6];  /* hardware address */
    uint8   valid;
    uint8   reserved;
    };
//--------------------------------------------------------------------------
//static struct arptab_s arptab[ARPTAB_SIZ];
//static uint32 arptab_next_available;

#define     BUF_FREE            0x00   /* Buffer is Free  */
#define     BUF_USED            0x80   /* Buffer is occupied */
#define     BUF_ASICHOLD        0x80   /* Buffer is hold by ASIC */
#define     BUF_DRIVERHOLD      0xc0   /* Buffer is hold by driver */

//--------------------------------------------------------------------------
/* mbuf header associated with each cluster 
*/
struct mBuf
{
    struct mBuf *m_next;
    struct pktHdr *m_pkthdr;            /* Points to the pkthdr structure */
	
#ifdef _LITTLE_ENDIAN
#ifdef CONFIG_RTL865XC
    uint16    m_flags;                  /* mbuf flags; see below */
#else
    int8      m_flags;                  /* mbuf flags; see below */
#endif
    uint16    m_len;                    /* data bytes used in this cluster */
#else	
    uint16    m_len;                    /* data bytes used in this cluster */
#ifdef CONFIG_RTL865XC
    uint16    m_flags;                  /* mbuf flags; see below */
#else
    int8      m_flags;                  /* mbuf flags; see below */
#endif
#endif

//    uint16    m_len;                    /* data bytes used in this cluster */
//#ifdef CONFIG_RTL865XC
//    uint16    m_flags;                  /* mbuf flags; see below */
//#else
//    int8      m_flags;                  /* mbuf flags; see below */
//#endif
#define MBUF_FREE            BUF_FREE   /* Free. Not occupied. should be on free list   */
#define MBUF_USED            BUF_USED   /* Buffer is occupied */
#define MBUF_EXT             0x10       /* has associated with an external cluster, this is always set. */
#define MBUF_PKTHDR          0x08       /* is the 1st mbuf of this packet */
#define MBUF_EOR             0x04       /* is the last mbuf of this packet. Set only by ASIC*/
    uint8     *m_data;                  /*  location of data in the cluster */
    uint8     *m_extbuf;                /* start of buffer*/
    //uint16    m_extsize;                /* sizeof the cluster */
   //int8      m_reserved[2];            /* padding */
#ifdef _LITTLE_ENDIAN
    int8      m_reserved[2];            /* padding */
    uint16    m_extsize;                /* sizeof the cluster */
#else	
    uint16    m_extsize;                /* sizeof the cluster */
    int8      m_reserved[2];            /* padding */
#endif

#ifdef ETH_NEW_FC
	void			*skb;
#endif
#ifdef TX_SCATTER
	struct list_head list;
	#ifdef _PKTHDR_CACHEABLE	/*keep cache line alianment, fix jwj*/
	uint32 padding0;
	uint32 padding1;
	uint32 padding2;
	uint32 padding3;
	uint32 padding4;
	uint32 padding5;
	uint32 padding6;
	#endif
#else
	#ifdef _PKTHDR_CACHEABLE
	uint32 padding0;
	#endif
#endif
};
//--------------------------------------------------------------------------
/* pkthdr records packet specific information. Each pkthdr is exactly 32 bytes.
 first 20 bytes are for ASIC, the rest 12 bytes are for driver and software usage.
*/
#if defined(CONFIG_RTL_8197F)
struct pktHdr
{
    union
    {
        struct pktHdr *pkthdr_next;     /*  next pkthdr in free list */
        struct mBuf *mbuf_first;        /*  1st mbuf of this pkt */
    }PKTHDRNXT;
#define ph_nextfree         PKTHDRNXT.pkthdr_next
#define ph_mbuf             PKTHDRNXT.mbuf_first

#ifdef _LITTLE_ENDIAN

    uint16    ph_srcExtPortNum: 2;      /* Both in RX & TX. Source extension port number. */
    uint16    ph_l2Trans: 1;            /* l2Trans - copy from HSA bit 129 */
    uint16    ph_isOriginal: 1;         /* isOriginal - DP included cpu port or more than one ext port */
    uint16    ph_hwFwd: 1;              /* hwFwd - copy from HSA bit 200 */
    uint16    ph_reserved2: 3;          /* reserved */
    uint16    ph_extPortList: 4;        /* dest extension port list. must be 0 for TX */
    uint16    ph_queueId: 3;            /* bit 2~0: Queue ID */
    uint16    ph_reserved1: 1;           /* reserved */
    uint16    ph_len;                   /*   total packet length */

#else
    uint16    ph_len;                   /*   total packet length */
    uint16    ph_reserved1: 1;           /* reserved */
    uint16    ph_queueId: 3;            /* bit 2~0: Queue ID */
    uint16    ph_extPortList: 4;        /* dest extension port list. must be 0 for TX */
    uint16    ph_reserved2: 3;          /* reserved */
    uint16    ph_hwFwd: 1;              /* hwFwd - copy from HSA bit 200 */
    uint16    ph_isOriginal: 1;         /* isOriginal - DP included cpu port or more than one ext port */
    uint16    ph_l2Trans: 1;            /* l2Trans - copy from HSA bit 129 */
    uint16    ph_srcExtPortNum: 2;      /* Both in RX & TX. Source extension port number. */
#endif

		/* for ph_extPortList */
		#define	PKTHDR_EXTPORT_MAGIC		0xA530
		#define	PKTHDR_EXTPORT_MAGIC2		0xA531
		#define	PKTHDR_EXTPORT_MAGIC3		0xA532
		#define	PKTHDR_EXTPORT_MAGIC4		0xA533
		#define	PKTHDR_EXTPORT_P1               6
		#define	PKTHDR_EXTPORT_P2               7
		#define	PKTHDR_EXTPORT_P3               8
		
		#define PKTHDR_EXTPORT_LIST_P0		0
		#define PKTHDR_EXTPORT_LIST_P1		1
		#define PKTHDR_EXTPORT_LIST_P2		2
		#define PKTHDR_EXTPORT_LIST_CPU		3
		#define PKTHDR_EXTPORTMASK_P0		(0x1 << (PKTHDR_EXTPORT_LIST_P0))
		#define PKTHDR_EXTPORTMASK_P1		(0x1 << (PKTHDR_EXTPORT_LIST_P1))
		#define PKTHDR_EXTPORTMASK_P2		(0x1 << (PKTHDR_EXTPORT_LIST_P2))
		#define PKTHDR_EXTPORTMASK_CPU		(0x1 << (PKTHDR_EXTPORT_LIST_CPU))
		#define PKTHDR_EXTPORTMASK_ALL		(	PKTHDR_EXTPORTMASK_P0 |\
												PKTHDR_EXTPORTMASK_P1 |\
												PKTHDR_EXTPORTMASK_P2 |\
												PKTHDR_EXTPORTMASK_CPU \
											)

#ifdef _LITTLE_ENDIAN
    uint16    ph_reason;                /* indicates wht the packet is received by CPU */

    uint16    ph_linkID: 7;             /* for WLAN WDS multiple tunnel */
    uint16    ph_pppoeIdx: 3;
    uint16    ph_pppeTagged: 1;         /* the tag status after ALE */
    uint16    ph_LLCTagged: 1;          /* the tag status after ALE */
    uint16    ph_vlanTagged: 1;         /* the tag status after ALE */
    uint16    ph_type: 3;
#else
    uint16    ph_type: 3;
    uint16    ph_vlanTagged: 1;         /* the tag status after ALE */
    uint16    ph_LLCTagged: 1;          /* the tag status after ALE */
    uint16    ph_pppeTagged: 1;         /* the tag status after ALE */
    uint16    ph_pppoeIdx: 3;
    uint16    ph_linkID: 7;             /* for WLAN WDS multiple tunnel */
    uint16    ph_reason;                /* indicates wht the packet is received by CPU */
#endif

#define	ph_proto				ph_type
#define PKTHDR_ETHERNET      0
#define PKTHDR_PPTP          1
#define PKTHDR_IP            2
#define PKTHDR_ICMP          3
#define PKTHDR_IGMP          4
#define PKTHDR_TCP           5
#define PKTHDR_UDP           6
#define PKTHDR_IPV6          7

#ifdef _LITTLE_ENDIAN
    uint8     ph_portlist;              /* RX: source port number, TX: destination portmask */
    uint8     ph_orgtos;                /* RX: original TOS of IP header's value before remarking, TX: undefined */
    uint16    ph_flags;                 /*  NEW:Packet header status bits */
#else
    uint16    ph_flags;                 /*  NEW:Packet header status bits */
    uint8     ph_orgtos;                /* RX: original TOS of IP header's value before remarking, TX: undefined */
    uint8     ph_portlist;              /* RX: source port number, TX: destination portmask */
#endif

#define PKTHDR_FREE          (BUF_FREE << 8)        /* Free. Not occupied. should be on free list   */
#define PKTHDR_USED          (BUF_USED << 8)
#define PKTHDR_ASICHOLD      (BUF_ASICHOLD<<8)      /* Hold by ASIC */
#define PKTHDR_DRIVERHOLD    (BUF_DRIVERHOLD<<8)    /* Hold by driver */
#define PKTHDR_CPU_OWNED     0x4000
#define PKT_INCOMING         0x1000     /* Incoming: packet is incoming */
#define PKT_OUTGOING         0x0800     /*  Outgoing: packet is outgoing */
#define PKT_BCAST            0x0100     /*send/received as link-level broadcast  */
#define PKT_MCAST            0x0080     /*send/received as link-level multicast   */
#define PKTHDR_BRIDGING      0x0040     /* when PKTHDR_HWLOOKUP is on. 1: Hardware assist to do L2 bridging only, 0:hardware assist to do NAPT*/
#define PKTHDR_HWLOOKUP      0x0020	/* valid when ph_extPortList!=0. 1:Hardware table lookup assistance*/
#define PKTHDR_PPPOE_AUTOADD    0x0004  /* PPPoE header auto-add */
#define CSUM_TCPUDP_OK       0x0001     /*Incoming:TCP or UDP cksum checked */
#define CSUM_IP_OK           0x0002     /* Incoming: IP header cksum has checked */
#define CSUM_TCPUDP          0x0001     /*Outgoing:TCP or UDP cksum offload to ASIC*/
#define CSUM_IP              0x0002     /* Outgoing: IP header cksum offload to ASIC*/


#ifdef _LITTLE_ENDIAN
	union
	{
		uint16	_flags2;			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
								/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
		struct
		{
			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
			uint16 _svlanId:12;			/* Source (Original) VLAN ID */
			uint16 _rxPktPriority:3;		/* Rx packet's original priority */
			uint16 _reserved:1;
		} _rx;

		struct
		{
			/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
			
			uint16 _txCVlanTagAutoAdd:6;	/* BitMask to indicate the port which would need to add VLAN tag */
			#if defined(CONFIG_RTL_8197F)
            uint16 _reserved:2;
			uint16 _q_priority_tx:3;            
			uint16 _reserved_2:5;

			#define ph_q_priority_tx	_flags2._tx._q_priority_tx
			#else
			uint16 _reserved:10;
			#endif            
		} _tx;
	} _flags2;

    uint16     ph_vlanId: 12;
    uint16     ph_txPriority: 3;
    uint16     ph_vlanId_resv: 1;
#else
   uint16     ph_vlanId_resv: 1;
   uint16     ph_txPriority: 3;
   uint16     ph_vlanId: 12;
	union
	{
		uint16	_flags2;			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
								/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
		struct
		{
			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
			uint16 _reserved:1;
			uint16 _rxPktPriority:3;		/* Rx packet's original priority */
			uint16 _svlanId:12;			/* Source (Original) VLAN ID */
		} _rx;

		struct
		{
			/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
			uint16 _reserved:10;
			uint16 _txCVlanTagAutoAdd:6;	/* BitMask to indicate the port which would need to add VLAN tag */
		} _tx;
	} _flags2;
#endif

	#define ph_dvlanId		ph_vlanId
	#define ph_rxPriority		ph_txPriority

	#define PKTHDR_TXPRIORITY_MIN			0
	#define PKTHDR_TXPRIORITY_MAX			7

	#define ph_flags2				_flags2._flags2
	#define ph_svlanId				_flags2._rx._svlanId
	#define ph_rxPktPriority			_flags2._rx._rxPktPriority
	#define ph_txCVlanTagAutoAdd	_flags2._tx._txCVlanTagAutoAdd

	#define PKTHDR_TXCVID(vid)					(vid & 0xfff)
	#define PKTHDR_VLAN_P0_AUTOADD			(0x0001<<0)
	#define PKTHDR_VLAN_P1_AUTOADD			(0x0001<<1)
	#define PKTHDR_VLAN_P2_AUTOADD			(0x0001<<2)
	#define PKTHDR_VLAN_P3_AUTOADD			(0x0001<<3)
	#define PKTHDR_VLAN_P4_AUTOADD			(0x0001<<4)
	#define PKTHDR_VLAN_P5_AUTOADD			(0x0001<<5)
	#define PKTHDR_VLAN_AUTOADD				(	(PKTHDR_VLAN_P0_AUTOADD)|	\
												(PKTHDR_VLAN_P1_AUTOADD)|	\
												(PKTHDR_VLAN_P2_AUTOADD)|	\
												(PKTHDR_VLAN_P3_AUTOADD)|	\
												(PKTHDR_VLAN_P4_AUTOADD)|	\
												(PKTHDR_VLAN_P5_AUTOADD)	)

#ifdef _LITTLE_ENDIAN
	#if defined(CONFIG_RTL_8197F)
	uint16	ph_ipIpv6HdrLen;	/* IPv6 header length */
	uint8	ph_ipIpv4_1st:1;	/* IPv4 first header */
	uint8	ph_ipIpv4:1;		/* IPv4 Header flag */
	uint8	ph_ipIpv6:1;		/* IPv6 Header flag */
	uint8	ph_ipResv:5;
	#else
	uint8	ph_reserved[3];		/* padding */
	#endif

	#if defined(CONFIG_RTL_8197F)
	uint8	ph_ptpPkt:1;		/* 1: PTP */
	uint8	ph_ptpVer:2;		/* PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
	uint8	ph_ptpMsgType:4;	/* message type */
	uint8	ph_ptpResv:1;
	#else
	uint8	ph_ptpreserved;
	#endif	
#else
	#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C)
	uint8	ph_ptpResv:1;
	uint8	ph_ptpMsgType:4;	/* message type */
	uint8	ph_ptpVer:2;		/* PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
	uint8	ph_ptpPkt:1;			/* 1: PTP */
	#else
	uint8	ph_ptpreserved;
	#endif

	#if defined(CONFIG_RTL_8198C)
	uint8	ph_ipResv:5;
	uint8	ph_ipIpv6:1;		/* IPv6 Header flag */
	uint8	ph_ipIpv4:1;		/* IPv4 Header flag */
	uint8	ph_ipIpv4_1st:1;	/* IPv4 first header */
	uint16	ph_ipIpv6HdrLen;	/* IPv6 header length */
	#else
	uint8	ph_reserved[3];		/* padding */
	#endif
#endif

#ifndef ETH_NEW_FC
	void			*skb;
#endif

#if defined(_PKTHDR_CACHEABLE)
	uint32 	pending0;	//for cache line alianment
	uint32	pending1;
#endif
};
#else
struct pktHdr
{
    union
    {
        struct pktHdr *pkthdr_next;     /*  next pkthdr in free list */
        struct mBuf *mbuf_first;        /*  1st mbuf of this pkt */
    }PKTHDRNXT;
#define ph_nextfree         PKTHDRNXT.pkthdr_next
#define ph_mbuf             PKTHDRNXT.mbuf_first
    uint16    ph_len;                   /*   total packet length */
    uint16    ph_reserved1: 1;           /* reserved */
    uint16    ph_queueId: 3;            /* bit 2~0: Queue ID */
    uint16    ph_extPortList: 4;        /* dest extension port list. must be 0 for TX */
    uint16    ph_reserved2: 3;          /* reserved */
    uint16    ph_hwFwd: 1;              /* hwFwd - copy from HSA bit 200 */
    uint16    ph_isOriginal: 1;         /* isOriginal - DP included cpu port or more than one ext port */
    uint16    ph_l2Trans: 1;            /* l2Trans - copy from HSA bit 129 */
    uint16    ph_srcExtPortNum: 2;      /* Both in RX & TX. Source extension port number. */

/* for ph_extPortList */
#define PKTHDR_EXTPORT_LIST_P0		0
#define PKTHDR_EXTPORT_LIST_P1		1
#define PKTHDR_EXTPORT_LIST_P2		2
#define PKTHDR_EXTPORT_LIST_CPU		3
#define PKTHDR_EXTPORTMASK_P0		(0x1 << (PKTHDR_EXTPORT_LIST_P0))
#define PKTHDR_EXTPORTMASK_P1		(0x1 << (PKTHDR_EXTPORT_LIST_P1))
#define PKTHDR_EXTPORTMASK_P2		(0x1 << (PKTHDR_EXTPORT_LIST_P2))
#if	!defined(CONFIG_RTL_HARDWARE_NAT)
#define PKTHDR_EXTPORTMASK_CPU		(0x1 << (PKTHDR_EXTPORT_LIST_CPU))
#endif
#define PKTHDR_EXTPORTMASK_ALL		(	PKTHDR_EXTPORTMASK_P0 |\
										PKTHDR_EXTPORTMASK_P1 |\
										PKTHDR_EXTPORTMASK_P2 |\
										PKTHDR_EXTPORTMASK_CPU \
									)

    uint16    ph_type: 3;
#define PKTHDR_ETHERNET      0
#define PKTHDR_IP            2
#define PKTHDR_ICMP          3
#define PKTHDR_IGMP          4
#define PKTHDR_TCP           5
#define PKTHDR_UDP           6
    uint16    ph_vlanTagged: 1;         /* the tag status after ALE */
    uint16    ph_LLCTagged: 1;          /* the tag status after ALE */
    uint16    ph_pppeTagged: 1;         /* the tag status after ALE */
    uint16    ph_pppoeIdx: 3;
    uint16    ph_linkID: 7;             /* for WLAN WDS multiple tunnel */
    uint16    ph_reason;                /* indicates wht the packet is received by CPU */

    uint16    ph_flags;                 /*  NEW:Packet header status bits */
#define PKTHDR_FREE          (BUF_FREE << 8)        /* Free. Not occupied. should be on free list   */
#define PKTHDR_USED          (BUF_USED << 8)
#define PKTHDR_ASICHOLD      (BUF_ASICHOLD<<8)      /* Hold by ASIC */
#define PKTHDR_DRIVERHOLD    (BUF_DRIVERHOLD<<8)    /* Hold by driver */
#define PKTHDR_CPU_OWNED     0x4000
#define PKT_INCOMING         0x1000     /* Incoming: packet is incoming */
#define PKT_OUTGOING         0x0800     /*  Outgoing: packet is outgoing */
#define PKT_BCAST            0x0100     /*send/received as link-level broadcast  */
#define PKT_MCAST            0x0080     /*send/received as link-level multicast   */
#define PKTHDR_BRIDGING      0x0040     /* when PKTHDR_HWLOOKUP is on. 1: Hardware assist to do L2 bridging only, 0:hardware assist to do NAPT*/
#define PKTHDR_HWLOOKUP      0x0020	/* valid when ph_extPortList!=0. 1:Hardware table lookup assistance*/
#define PKTHDR_PPPOE_AUTOADD    0x0004  /* PPPoE header auto-add */
#define CSUM_TCPUDP_OK       0x0001     /*Incoming:TCP or UDP cksum checked */
#define CSUM_IP_OK           0x0002     /* Incoming: IP header cksum has checked */
#define CSUM_TCPUDP          0x0001     /*Outgoing:TCP or UDP cksum offload to ASIC*/
#define CSUM_IP              0x0002     /* Outgoing: IP header cksum offload to ASIC*/

   uint8      ph_orgtos;                /* RX: original TOS of IP header's value before remarking, TX: undefined */
   uint8      ph_portlist;              /* RX: source port number, TX: destination portmask */

   uint16     ph_vlanId_resv: 1;
   uint16     ph_txPriority: 3;
   uint16     ph_vlanId: 12;
   uint16     ph_flags2;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
   uint8      ph_ptpResv:1;
   uint8      ph_ptpMsgType:4;	/* message type */
   uint8      ph_ptpVer:2;	/* PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
   uint8      ph_ptpPkt:1;	/* 1: PTP */
   int8       ph_reserved[3];            /* padding */
#endif
   
#ifndef ETH_NEW_FC
	void			*skb;
#endif
	#ifdef _PKTHDR_CACHEABLE	/*keep cache line alianment, fix jwj*/
	uint32 padding0;
	uint32 padding1;
	#endif
};
#endif

#ifdef TX_SCATTER
static struct list_head mbuf_que;
#endif

#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
unsigned int rx_noBuffer_cnt=0;
unsigned int rx_noBuffer_cnt1=0;
unsigned int tx_ringFull_cnt=0;
#endif

//--------------------------------------------------------------------------
__IRAM_SECTION_
static void save_and_cli2(unsigned long flags)
{
	save_and_cli(flags);
}

__IRAM_SECTION_
static void restore_flags2(unsigned long flags)
{
	restore_flags(flags);
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
//static void arpInput(uint8*,uint32);
//static int32 arpResolve(uint8*,uint8*);


//#pragma ghs section text=".iram"
/*************************************************************************
*   FUNCTION                                                              
*       swNic_intHandler                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function is the handler of NIC interrupts
*                                                                         
*   INPUTS                                                                
*       intPending      Pending interrupt sources.
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
/*void swNic_intHandler(uint32 intPending) {return;}*/

#if defined(CONFIG_RTK_VOIP) || defined(DBG_DESC)
void print_desc_own_bit(void)
{
	int idx;
	
	printk("   ");
	for (idx=0; idx<10; idx++) {
		printk(".%d-%d.", idx, idx);
	}
	printk("\n\n");
	
	for (idx=0; idx<rxPkthdrRingCnt[0]; idx++) {
		if(idx %10 ==0 )
			printk("%2d ", idx/10);
		printk(".%d-%d.", (rxPkthdrRing[0][idx] & 3), (rxMbufRing[idx] & 3));
		if(idx %10 ==9)
			printk("\n");
	}
}

void print_desc_own_bit2(void)
{
	int index, index2; 
	index= 
        ( 
        	(
        	(REG32(CPURPDCR0)-(uint32)rxPkthdrRing[0])
        	&~(DESC_OWNED_BIT | DESC_WRAP)
        	)
        /4
        );
        index2 = 
        ( 
        	(
        	(REG32(CPURMDCR0)-(uint32)rxMbufRing)
        	&~(DESC_OWNED_BIT | DESC_WRAP)
        	)
        	/4
        );
	printk("(%d ) (%d, %d)", currRxPkthdrDescIndex, index, index2);
}

void dump_rx_desc_own_bit(void)
{
	int idx;

	printk("rx desc: (total %d)\n", rxPkthdrRingCnt[0]);
	for (idx=0; idx<rxPkthdrRingCnt[0]; idx++) {
		printk(".%d-%d.", (rxPkthdrRing[0][idx] & 3), (rxMbufRing[idx] & 3));
	}

#ifdef DELAY_REFILL_ETH_RX_BUF
	printk("\n(currRX: %d, readyForHw: %d, prevMbufUsed %d)\n", 
		currRxPkthdrDescIndex, rxDescReadyForHwIndex, currRxMbufDescIndex);
#else
	printk("\n(currRX: %d, prevMbufUsed %d)\n", 
		currRxPkthdrDescIndex, currRxMbufDescIndex);
#endif
}

void dump_tx_desc_own_bit(void)
{
	int idx;

	printk("tx desc: (total %d)\n", txPkthdrRingCnt[0]);
	for (idx=0; idx<txPkthdrRingCnt[0]; idx++) {
		printk(".%d.", (txPkthdrRing[0][idx] & 3));
	}

	printk("\n(curr: %d, txDone: %d)\n", currTxPkthdrDescIndex, txPktDoneDescIndex);
}
#endif

#ifdef DELAY_REFILL_ETH_RX_BUF
inline int buffer_reuse(int index1, int index2) 
{
	int gap = (index2 > index1) ? (index2 - index1) : (index2 + rxPkthdrRingCnt[0] - index1);
	
	if ((rxPkthdrRingCnt[0] - gap) < ETH_REFILL_THRESHOLD)
		return 1;
	else
		return 0;
}

inline void set_RxPkthdrRing_OwnBit(void) 
{
	rxPkthdrRing[0][rxDescReadyForHwIndex] |= DESC_SWCORE_OWNED;
	
	if ( ++rxDescReadyForHwIndex == rxPkthdrRingCnt[0] )
		rxDescReadyForHwIndex = 0;
}


#ifdef CONFIG_RTL_DELAY_REFILL
/*
	return value: 1 ==> success, returned to rx pkt hdr desc
	return value: 0 ==> failed, no return ==> release to priv skb buf pool
 */	
 /*now always return 0 since we re-alloc one skb*/
int return_to_rx_pkthdr_ring(unsigned char *head) 
{
	unsigned long flags;
	struct sk_buff *skb;
	struct pktHdr *pReadyForHw;
	int ret=0;
	uint32 mbufIndex;
#ifndef ETH_NEW_FC
	struct pktHdr *alignWithMbuf;
#endif
	
	save_and_cli(flags);
	
	if (rxDescReadyForHwIndex != currRxPkthdrDescIndex) {

		if(NULL==alloc_rx_buf(&skb, size_of_cluster))
			goto _ret1;
		if(skb == NULL)
			goto _ret1;

		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		
		//diag_printf("%s %d skb %p skb->head %p skb->data %p\n",__FUNCTION__,__LINE__,
			//skb,skb->head, skb->data);
#if 0		
		skb_reserve(skb, RX_OFFSET);

#ifndef ETH_NEW_FC
		#ifdef RTL_ETH_RX_RUNOUT
		/* store the skb pointer in a DW in front of  new_skb->data, it will be used in swNic_receive() */
		*(uint32 *)(skb->data-6) = (uint32)(skb);
		#endif
#endif
#endif
		pReadyForHw = (struct pktHdr *)(rxPkthdrRing[0][rxDescReadyForHwIndex] & 
						~(DESC_OWNED_BIT | DESC_WRAP));    
		mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct mBuf));

		pReadyForHw->ph_mbuf->m_data = skb->data;
		pReadyForHw->ph_mbuf->m_extbuf = skb->data;
#ifdef ETH_NEW_FC
		pReadyForHw->ph_mbuf->skb = (void *)skb;
#else
		alignWithMbuf = (struct pktHdr *)(rxPkthdrRing[0][mbufIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
		alignWithMbuf->skb = skb;
#endif		
		_dma_cache_wback_inv((unsigned long)skb->head, size_of_cluster);
		rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;

#ifdef ETH_NEW_FC  // DBG_DESC
		currRxMbufDescIndex = mbufIndex;
#endif

		set_RxPkthdrRing_OwnBit();

		ret = 0;
	}

_ret1:
	restore_flags(flags);
	return ret;
}

#else
/*
	return value: 1 ==> success, returned to rx pkt hdr desc
	return value: 0 ==> failed, no return ==> release to priv skb buf pool
 */	
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
int return_to_rx_pkthdr_ring(unsigned char *head) 
{
	unsigned long flags;
	struct sk_buff *skb;
	struct pktHdr *pReadyForHw;
	int ret=0;
	uint32 mbufIndex;
#ifndef ETH_NEW_FC
	struct pktHdr *alignWithMbuf;
#endif
	
	save_and_cli(flags);
	
	if (rxDescReadyForHwIndex != currRxPkthdrDescIndex) {

		skb = dev_alloc_8190_skb(head, CROSS_LAN_MBUF_LEN);
		if (skb == NULL)
			goto _ret1;

		skb_reserve(skb, RX_OFFSET);

#ifndef ETH_NEW_FC
		#ifdef RTL_ETH_RX_RUNOUT
		/* store the skb pointer in a DW in front of  new_skb->data, it will be used in swNic_receive() */
		*(uint32 *)(skb->data-6) = (uint32)(skb);
		#endif
#endif
		pReadyForHw = (struct pktHdr *)(rxPkthdrRing[0][rxDescReadyForHwIndex] & 
						~(DESC_OWNED_BIT | DESC_WRAP));    
		mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct mBuf));

		pReadyForHw->ph_mbuf->m_data = skb->data;
		pReadyForHw->ph_mbuf->m_extbuf = skb->data;
#ifdef ETH_NEW_FC
		pReadyForHw->ph_mbuf->skb = (void *)skb;
#else
		alignWithMbuf = (struct pktHdr *)(rxPkthdrRing[0][mbufIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
		alignWithMbuf->skb = skb;
#endif		
		_dma_cache_wback_inv((unsigned long)skb->head, skb->truesize);
		rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;

#ifdef ETH_NEW_FC  // DBG_DESC
		currRxMbufDescIndex = mbufIndex;
#endif

		set_RxPkthdrRing_OwnBit();

		ret = 1;
	}

_ret1:
	restore_flags(flags);
	return ret;
}
#endif
#endif

/*************************************************************************
*   FUNCTION                                                              
*       swNic_receive                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function reads one packet from rx descriptors, and return the 
*       previous read one to the switch core. This mechanism is based on 
*       the assumption that packets are read only when the handling 
*       previous read one is done.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
//#ifndef __ECOS
__MIPS16
//#endif
__IRAM_SECTION_
int32 swNic_receive(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid)
{
	struct pktHdr * pPkthdr;
	unsigned char *buf;
	void *skb;
	unsigned long flags=0;
	
#ifdef RTL_ETH_RX_RUNOUT
	uint32 rxMbufDescIndex;
#endif

#ifdef DELAY_REFILL_ETH_RX_BUF
	struct pktHdr *pReadyForHw;
	uint32 mbufIndex;	
#ifndef ETH_NEW_FC
	struct pktHdr *alignWithMbuf;
#endif
#endif
	
	//save_and_cli(flags);
	save_and_cli2(flags);
	
get_next:
	/* Check OWN bit of descriptors */
	if ((rxPkthdrRing[0][currRxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED ) {   
		/* Fetch pkthdr */
		#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		pPkthdr = (struct pktHdr *) (rxPkthdrRing_base[0] + (sizeof(struct pktHdr) * currRxPkthdrDescIndex));								
		#else
		pPkthdr = (struct pktHdr *) (rxPkthdrRing[0][currRxPkthdrDescIndex] & 
                                            ~(DESC_OWNED_BIT | DESC_WRAP));    
		#endif
		/* Increment counter */
		rxPktCounter++;

#ifdef _PKTHDR_CACHEABLE
		_dma_cache_wback_inv((unsigned long)pPkthdr, sizeof(struct pktHdr));
		_dma_cache_wback_inv((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct mBuf));
#endif

		#ifdef RTL_ETH_RX_RUNOUT		
		rxMbufDescIndex = ((uint32)(pPkthdr->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct mBuf));
		#endif

		#if defined(CONFIG_RTL865X_HW_TABLES) // ||defined(CONFIG_RTL_HARDWARE_NAT)
		//extern int rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid);
		if (rtl8651_rxPktPreprocess(pPkthdr, vid) != 0) {
			buf = NULL;
		}
		else {
			buf = alloc_rx_buf(&skb, size_of_cluster);
		}
		#else		
		buf = alloc_rx_buf(&skb, size_of_cluster);
		#endif
		
		if (buf) {
			_dma_cache_wback_inv((unsigned long)buf, size_of_cluster); //michael
			
			#ifdef RTL_ETH_RX_RUNOUT 
			/*
			 * Note: fix the case when rx descriptor runout occurred.
			 *
			 * these two uncached pointer pPkthdr and pPkthdr->ph_mbuf are updated by hardware
			 * when a packet is received.
			 * if no any rx descriptor runout occurred, pPkthdr and pPkthdr->ph_mbuf will pointer to 
			 * next struct accordingly.
			 * but when rx descriptor runout occurred, pPkthdr->ph_mbuf is not pointed to next mbuf
			 * struct (I still do not know why?). it may point to next 5 or 6 mbuf struct pointer.
			 * the old code "*input = pPkthdr->skb" will return the wrong skb pointer to caller, so I store the skb 
			 * pointer in front of m_data (done in alloc_rx_buf()) and retrieve it here to send it to the caller.
			 */
#ifdef ETH_NEW_FC
			*input = pPkthdr->ph_mbuf->skb;
#else
			*input = (void *)(*(uint32 *)(pPkthdr->ph_mbuf->m_data - 6));
#endif
			#else
			*input = pPkthdr->skb;
			#endif
			
			*pLen = pPkthdr->ph_len - 4;				
			//_dma_cache_wback_inv((unsigned long)pPkthdr->ph_mbuf->m_data, *pLen);

			/*
			 * vid is assigned in rtl8651_rxPktPreprocess() 
			 * do not update it when CONFIG_RTL865X_HW_TABLES is defined
			 */
			#if !(defined(CONFIG_RTL865X_HW_TABLES)&&defined(CONFIG_RTL_HARDWARE_NAT))
			*vid=pPkthdr->ph_vlanId;
			#endif
			//hx-20120831
			//*pid=1<<pPkthdr->ph_portlist;
			#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			*pid=(pPkthdr->ph_portlist&0x7);
			#else
			*pid=pPkthdr->ph_portlist;
			#endif

			#ifdef DELAY_REFILL_ETH_RX_BUF
			pReadyForHw = (struct pktHdr *)(rxPkthdrRing[0][rxDescReadyForHwIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
			mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /(sizeof(struct mBuf));

			pReadyForHw->ph_mbuf->m_data = buf;
			pReadyForHw->ph_mbuf->m_extbuf = buf;
#ifdef ETH_NEW_FC
			pReadyForHw->ph_mbuf->skb = skb;
#else
			alignWithMbuf = (struct pktHdr *)(rxPkthdrRing[0][mbufIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
			alignWithMbuf->skb = skb;
#endif
			rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;	

#ifdef ETH_NEW_FC  // DBG_DESC
			currRxMbufDescIndex = mbufIndex;
#endif			
			set_RxPkthdrRing_OwnBit();
			
			#else
			pPkthdr->ph_mbuf->m_data = pPkthdr->ph_mbuf->m_extbuf = buf;				
#ifdef ETH_NEW_FC
			pPkthdr->ph_mbuf->skb = skb;
#else
			pPkthdr->skb = skb;
#endif
			#endif

			#if defined(CONFIG_RTK_VOIP_WAN_VLAN)|| defined(CONFIG_RTK_VOIP_865xC_QOS)
			struct sk_buff *sk;
			sk = *input;
			sk->rx_port = *pid;
			#endif
		}

		#ifdef DELAY_REFILL_ETH_RX_BUF
		else if (!buffer_reuse(rxDescReadyForHwIndex, (currRxPkthdrDescIndex+1))) {
			#ifdef RTL_ETH_RX_RUNOUT 
#ifdef ETH_NEW_FC
			*input = pPkthdr->ph_mbuf->skb;
#else
			*input = (void *)(*(uint32 *)(pPkthdr->ph_mbuf->m_data - 6));
#endif
			#else
			*input = pPkthdr->skb;
			#endif
			*pLen = pPkthdr->ph_len - 4;				
			//_dma_cache_wback_inv((unsigned long)pPkthdr->ph_mbuf->m_data, *pLen); 

			#ifndef CONFIG_RTL865X_HW_TABLES
			*vid=pPkthdr->ph_vlanId;
			#endif
			*pid=1<<pPkthdr->ph_portlist;

			buf = (unsigned char *)*input; // just only for "if (buf == NULL)" below
#ifdef ETH_NEW_FC
			/* we do not free this skbuff in swNic_freeRxBuf() */
			pPkthdr->ph_mbuf->skb = NULL; 
#endif
		}
		else {
			#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
				rx_noBuffer_cnt++;
			#endif
#if 0 //def ETH_NEW_FC
			/*	2009-11-25
				when encounter the reused case, we do not receive(and drop) this packet and
				keep it in rx descriptor. it makes the chance of triggering the hardware flow control bigger.
			 */
			reused_skb_num++;			
			return -1;
#else			
			// re-link skb and buffer pointer to the index "rxDescReadyForHwIndex"
			#ifdef RTL_ETH_RX_RUNOUT
#ifdef ETH_NEW_FC
			skb = pPkthdr->ph_mbuf->skb;
#else
			skb = (void *)(*(uint32 *)(pPkthdr->ph_mbuf->m_data - 6));
#endif
			#else
			skb = pPkthdr->skb;
			#endif

			pReadyForHw = (struct pktHdr *)(rxPkthdrRing[0][rxDescReadyForHwIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
			mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /(sizeof(struct mBuf));

			pReadyForHw->ph_mbuf->m_data = ((struct sk_buff *)skb)->data;
			pReadyForHw->ph_mbuf->m_extbuf = ((struct sk_buff *)skb)->data;
#ifdef ETH_NEW_FC
			pReadyForHw->ph_mbuf->skb = skb;
#else
			alignWithMbuf = (struct pktHdr *)(rxPkthdrRing[0][mbufIndex] & ~(DESC_OWNED_BIT | DESC_WRAP));    
			alignWithMbuf->skb = skb;
#endif
			rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;	
			
#ifdef ETH_NEW_FC  // DBG_DESC
			currRxMbufDescIndex = mbufIndex;
#endif
			set_RxPkthdrRing_OwnBit();
#endif			
		}		
		#else

		#ifdef RTL_ETH_RX_RUNOUT 
		/* 
		 * inherit the Note above,
		 * if rx descriptor runout occurred, pPkthdr->ph_mbuf is pointed to next 5 or 6 mbuf struct pointer.
		 * these own bits of mbuf ring between currRxMbufDescIndex (previous mbuf index which own bit be set by driver) 
		 * and rxMbufDescIndex (the actual mbuf index of this received packet which calculate through pPkthdr->ph_mbuf) 
		 * must be set to switch-owned.
		 */
		if ((rxMbufDescIndex < rxMbufRingCnt)) {
			int i;
			if (rxMbufDescIndex >= currRxMbufDescIndex) {
				for (i=currRxMbufDescIndex; i<=rxMbufDescIndex; i++)
					rxMbufRing[i] |= DESC_SWCORE_OWNED;
			}
			else {
				for (i=currRxMbufDescIndex; i<rxMbufRingCnt; i++)
					rxMbufRing[i] |= DESC_SWCORE_OWNED;
				for (i=0; i<=rxMbufDescIndex; i++)
					rxMbufRing[i] |= DESC_SWCORE_OWNED;
			}			
			currRxMbufDescIndex = rxMbufDescIndex;
		}
		else {
			rxMbufRing[currRxMbufDescIndex] |= DESC_SWCORE_OWNED;
		}
		rxPkthdrRing[0][currRxPkthdrDescIndex] |= DESC_SWCORE_OWNED;

#ifdef _PKTHDR_CACHEABLE
		_dma_cache_wback_inv((unsigned long)rxPkthdrRing[0][currRxPkthdrDescIndex], sizeof(struct pktHdr));
		_dma_cache_wback_inv((unsigned long)rxMbufRing[rxMbufDescIndex], sizeof(struct mBuf));
#endif
		#else
		rxPkthdrRing[0][currRxPkthdrDescIndex] |= DESC_SWCORE_OWNED;
		rxMbufRing[currRxMbufDescIndex] |= DESC_SWCORE_OWNED;
		
#ifdef _PKTHDR_CACHEABLE
		_dma_cache_wback_inv((unsigned long)rxPkthdrRing[0][currRxPkthdrDescIndex], sizeof(struct pktHdr));
		_dma_cache_wback_inv((unsigned long)rxMbufRing[currRxMbufDescIndex], sizeof(struct mBuf));
#endif

		#endif
		if ( ++currRxMbufDescIndex == rxMbufRingCnt )
			currRxMbufDescIndex = 0;

		#endif
		
		/* Increment index */
		if ( ++currRxPkthdrDescIndex == rxPkthdrRingCnt[0] )
			currRxPkthdrDescIndex = 0;

		if (buf == NULL) {
			#if defined(CONFIG_RTL_RXTX_STATS_DEBUG)
				rx_noBuffer_cnt1++;
			#endif
			#ifdef CONFIG_RTL865X_ETH_PRIV_SKB			
			reused_skb_num++;			
			#endif
			
			goto get_next;
		}

#ifdef __KERNEL__
		// Verify IP/TCP/UDP checksum
		if ((pPkthdr->ph_flags & (CSUM_TCPUDP_OK | CSUM_IP_OK)) != (CSUM_TCPUDP_OK | CSUM_IP_OK))
			((struct sk_buff *)*input)->ip_summed = 0xff;
		else
			((struct sk_buff *)*input)->ip_summed = 0;
#endif

		//restore_flags(flags);
		restore_flags2(flags);
		return 0;
	}
	else {
		//restore_flags(flags);
		restore_flags2(flags);
		return -1;
	}
}

/*************************************************************************
*   FUNCTION                                                              
*       swNic_Desp_RouOut                                        
*                                                                         
*   DESCRIPTION                                                           
*       This function check tx descriptors run out.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
#ifndef __ECOS
int swNic_Desp_RouOut(void)
{
	int next_index;
	unsigned long flags;

	save_and_cli(flags);
	if ((currTxPkthdrDescIndex+1) == txPkthdrRingCnt[0])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex+1;
	if (next_index == txPktDoneDescIndex) {
		//printk("Tx Desc full!\n");
		restore_flags(flags);
		return 1;
	}
	restore_flags(flags);	
	return 0;
}
#endif

#if defined(__ECOS)&&!defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
int can_send(void)
{
	int32 next_index;
	
	if ((currTxPkthdrDescIndex+1) == txPkthdrRingCnt[0])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex+1;

	if (next_index == txPktDoneDescIndex) {
		RTL_swNic_txDone();
		if (next_index == txPktDoneDescIndex) 	
			return 0;
	}

	if (txPktDoneDescIndex > next_index)
		return (txPktDoneDescIndex - next_index);
	else
		return (txPkthdrRingCnt[0] - next_index + txPktDoneDescIndex);
}
#endif


/*************************************************************************
*   FUNCTION                                                              
*       swNic_send                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function writes one packet to tx descriptors, and waits until 
*       the packet is successfully sent.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/

#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
extern int oldStatus;
#endif

//#ifndef __ECOS
__MIPS16
//#endif
__IRAM_SECTION_
int32 swNic_send(void *skb, void * output, uint32 len,unsigned int vid, unsigned int pid)
{
	struct pktHdr * pPkthdr;
	int next_index, ret;
	unsigned long flags=0;
#ifdef CONFIG_RTK_VOIP_WAN_VLAN
	static u32 arp_choice = 0;
#endif
	struct sk_buff* sk = skb;
#ifdef TX_SCATTER
	int padding_len;
#endif
	//save_and_cli(flags);
	save_and_cli2(flags);
	
	if ((currTxPkthdrDescIndex+1) == txPkthdrRingCnt[0])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex+1;

#ifdef ETH_NEW_FC
	if (next_index == txPktDoneDescIndex) {
		/* when tx descriptor is full, we call swNic_txDone() to return own bit if any */
		RTL_swNic_txDone();
		if (next_index == txPktDoneDescIndex) {
			//restore_flags(flags);
			restore_flags2(flags);
			return -1;
		}
	}		
#else
	if (next_index == txPktDoneDescIndex) {
//		printk("Tx Desc full!\n");
		//restore_flags(flags);
		restore_flags2(flags);
		return -1;
	}		
#endif

	/* Fetch packet header from Tx ring */
	pPkthdr = (struct pktHdr *) ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

	/* Pad small packets and add CRC */
	if ( len < 60 )
		pPkthdr->ph_len = 64;
	else
		pPkthdr->ph_len = len + 4;
		
#ifdef TX_SCATTER
	padding_len = ((uint32)pPkthdr->ph_len) - len;
#else
	pPkthdr->ph_mbuf->m_len  = pPkthdr->ph_len;
	pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;
#endif	

#ifdef ETH_NEW_FC
	pPkthdr->ph_mbuf->skb  = skb;
#else
	pPkthdr->skb = skb;
#endif

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
	pPkthdr->ph_portlist = pid & 0x3F;
	if (pid & 0x80 /* BIT(7) */) {
		pPkthdr->ph_portlist |= 0x40 /* BIT(6) */;
		pPkthdr->ph_srcExtPortNum = 0x2;
		///- pPkthdr->ph_type = PKTHDR_IP;
		pPkthdr->ph_flags |= PKTHDR_HWLOOKUP;
	}
	if (((struct sk_buff *)skb)->ip_summed == CHECKSUM_HW) {
		pPkthdr->ph_type = PKTHDR_IP;
		pPkthdr->ph_flags |= CSUM_IP;
	}
#elif (defined(CONFIG_RTL8197B_PANA) || defined(CONFIG_RTL865X_PANAHOST))
	pPkthdr->ph_portlist = pid;

#elif defined(CONFIG_RTL8196_RTL8366)
	pPkthdr->ph_portlist = 0x1;
	pPkthdr->ph_flags=(PKTHDR_USED|PKT_OUTGOING);
	pPkthdr->ph_srcExtPortNum = 0;

#elif defined(CONFIG_POCKET_ROUTER_SUPPORT)
	pPkthdr->ph_portlist = (pid & PORT_HW_AP_MODE);
#else
	pPkthdr->ph_portlist = pid;
	pPkthdr->ph_srcExtPortNum = 0;
	//diag_printf("pPkthdr->ph_portlist:%x,sk->cb[0]:%x,[%s]:[%d].\n",pPkthdr->ph_portlist,sk->cb[0],__FUNCTION__,__LINE__);
// -- Modify for IGMP snooping for Ethernet port ------------
//	if((sk->data[0]&0x01)==0)
	if (sk->data[0]&0x01) {
		//if (sk->cb[0]) 
		{
			pPkthdr->ph_portlist = pid;
		}
		
		#ifdef CONFIG_HW_PROTOCOL_VLAN_TBL
		if ((old_passThru_flag & IP6_PASSTHRU_MASK) && 
			!((sk->data[0]==0x01) && (sk->data[1]==0x00) && (sk->data[2]==0x5e)))
		{
			if ((*((uint16*)&sk->data[12])==htons(0x86dd)))
			{
				pPkthdr->ph_portlist = RTL8651_CPU_PORT;		/* must be set 0x7 */
				pPkthdr->ph_srcExtPortNum = PKTHDR_EXTPORT_LIST_P1;
				pPkthdr->ph_flags |= (PKTHDR_HWLOOKUP);
			}
		}
		#endif
		
	}
	else
//----------------------------------- david+2008-11-05	
	{
		/* unicast process */
		if ((vid == LAN_VID)
		#if defined(CONFIG_RTK_GUEST_ZONE) || defined(CONFIG_RTK_VLAN_SUPPORT)
			&& (L2_table_disabled == 0) 
		#endif
		#if defined(CONFIG_RTL_VLAN_SUPPORT)
			&& (rtl_vlan_support_enable == 0) 
		#endif
			)
		{
			/* the pkt must be tx to lan vlan */
			pPkthdr->ph_portlist = RTL8651_CPU_PORT;		/* must be set 0x7 */

			/*
			     pPkthdr->ph_srcExtPortNum = 1, vlan table's bit 6 must be set
			     pPkthdr->ph_srcExtPortNum = 2, vlan table's bit 7 must be set
			     pPkthdr->ph_srcExtPortNum = 3, vlan table's bit 8 must be set
			 */
			pPkthdr->ph_srcExtPortNum = PKTHDR_EXTPORT_LIST_P1;
			pPkthdr->ph_flags |= (PKTHDR_HWLOOKUP|PKTHDR_BRIDGING);
		}
	}
#endif
	/* Set cluster pointer to buffer */		
#ifdef TX_SCATTER
	if (sk->list_num == 1) {
		pPkthdr->ph_mbuf->m_len  = pPkthdr->ph_len;
		pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;
		pPkthdr->ph_mbuf->m_data    = output;
		pPkthdr->ph_mbuf->m_extbuf = output;
		pPkthdr->ph_mbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
		pPkthdr->ph_mbuf->m_next = NULL;
	}
	else if (sk->list_num > 1) {
		struct mBuf *mbuf= NULL, *mbuf_prev = NULL;
		struct list_head *plist;
		int i;
		
		pPkthdr->ph_mbuf->m_len  = sk->list_buf[0].len;
		pPkthdr->ph_mbuf->m_extsize = sk->list_buf[0].len;
		pPkthdr->ph_mbuf->m_data    = sk->list_buf[0].buf;
		pPkthdr->ph_mbuf->m_extbuf = sk->list_buf[0].buf;
		pPkthdr->ph_mbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR;
		mbuf_prev = pPkthdr->ph_mbuf;
		for (i=1; i<sk->list_num; i++) {
			if (list_empty(&mbuf_que)) {
				diag_printf("%s, mbuf_que empty!\n", __FUNCTION__); // for debug
				//restore_flags(flags);
				restore_flags2(flags);
				return -1;
			}
			
			plist = mbuf_que.next;
			list_del_init((struct list_head *)plist);			
			mbuf = list_entry(plist, struct mBuf, list);
	
			mbuf->m_data    = sk->list_buf[i].buf;
			mbuf->m_extbuf = sk->list_buf[i].buf;
			mbuf->m_len = sk->list_buf[i].len;
			mbuf->m_extsize = sk->list_buf[i].len;
			mbuf->m_pkthdr = pPkthdr;
			
			mbuf->m_flags = MBUF_USED | MBUF_EXT;
			mbuf->skb  = NULL;	
			mbuf_prev->m_next = mbuf;
			
			mbuf_prev = mbuf;
		}
		
		mbuf->m_len += padding_len;
		mbuf->m_extsize += padding_len;
		mbuf->m_next = NULL;
		mbuf->m_flags |= MBUF_EOR;
	}
	else {
		diag_printf("error, should not go here!\n");
		//restore_flags(flags);
		restore_flags2(flags);
		return -1;
	}
#else
	pPkthdr->ph_mbuf->m_data    = output;
	pPkthdr->ph_mbuf->m_extbuf = output;
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
	pPkthdr->ph_ptpPkt = 0;
#endif

	ret = currTxPkthdrDescIndex;
	currTxPkthdrDescIndex = next_index;

#ifdef CONFIG_RTK_VOIP_865xC_QOS
		
		if(sk->rx_port&(1<<6))
		{
			pPkthdr->ph_txPriority = DEFAULT_VOICE_PRIORITY;
		}
		else 
		{
			pPkthdr->ph_txPriority = DEFAULT_DATA_PRIORITY;
		}
#endif

#ifdef CONFIG_RTK_VOIP_WAN_VLAN
		{
		struct mBuf *mbuf;
		uint8 priorityField;
		mbuf = pPkthdr->ph_mbuf;
		if(wan_vlan_enable)
		{
			if (vid == DEFAULT_WAN_VLAN_ID) // WAN port only!
			{	
				if (skb_headroom(skb) < 4 && skb_cow(skb, 4) !=0 )
				{
					printk("%s-%d: error! (skb_headroom(skb) == %d < 4). Enlarge it!\n",
					__FUNCTION__, __LINE__, skb_headroom(skb));
					while (1) ;
				}
				skb_push(skb, 4);
				
				mbuf->m_data -= 4; // skb_headroom(skb) must > 4
				mbuf->m_len += 4;
				pPkthdr->ph_len += 4;
				memmove(mbuf->m_data, mbuf->m_data+4, 2*sizeof(ether_addr_t));	
				
				if(sk->rx_port&(1<<6))//CPU
				{
					if(*((uint16 *)(&(mbuf->m_data[16]))) == htons(0x0806))//ARP
					{
						switch (arp_choice%3) 
						{
						case 0:
							pPkthdr->ph_vlanId = wan_vlan_id_proto;
							pPkthdr->ph_txPriority = wan_priority_proto;	
							priorityField = (uint8)wan_priority_proto << 5;	// just get most-significant 3 bits 
							break;
						case 1:
							pPkthdr->ph_vlanId = wan_vlan_id_video;
							pPkthdr->ph_txPriority = wan_priority_video;	
							priorityField = (uint8)wan_priority_video << 5;	// just get most-significant 3 bits 
							break;
						case 2:
							pPkthdr->ph_vlanId = wan_vlan_id_data;
							pPkthdr->ph_txPriority = wan_priority_data;	
							priorityField = (uint8)wan_priority_data << 5;	// just get most-significant 3 bits 
							break;
						}
						++arp_choice;
					}
					else
					{
					pPkthdr->ph_vlanId = wan_vlan_id_proto;
					pPkthdr->ph_txPriority = wan_priority_proto;	
					priorityField = (uint8)wan_priority_proto << 5;	// just get most-significant 3 bits 
							//printk("in swNic_send , local gene\n");
					}
					*((uint16*)(&(mbuf->m_data[12]))) = htons(0x8100);
					*((uint16*)(&(mbuf->m_data[14]))) = htons(pPkthdr->ph_vlanId);
					*((uint8 *)(&(mbuf->m_data[14]))) &=0x0f ;
					*((uint8 *)(&(mbuf->m_data[14]))) |= priorityField;
				}
			#ifdef CONFIG_FIX_WAN_TO_4
				else if(sk->rx_port==(1<<3))
			#else
				else if(sk->rx_port==(1<<1))//Video Port
			#endif
				{	
					pPkthdr->ph_vlanId = wan_vlan_id_video;//vid;	
					pPkthdr->ph_txPriority = wan_priority_video;
					priorityField = (uint8)wan_priority_video << 5;	// just get most-significant 3 bits 			
					
					*((uint16*)(&(mbuf->m_data[12]))) = htons(0x8100);
					*((uint16*)(&(mbuf->m_data[14]))) = htons(wan_vlan_id_video);
					*((uint8 *)(&(mbuf->m_data[14]))) &=0x0f ;
					*((uint8 *)(&(mbuf->m_data[14]))) |= htons(priorityField);
					//printk("in swNic_send , Video gene, vlan: %d, priority: %d\n", pPkthdr->ph_vlanId, pPkthdr->ph_txPriority);
				}
				else
				{
					pPkthdr->ph_vlanId = wan_vlan_id_data;//vid;
					pPkthdr->ph_txPriority = wan_priority_data;
					priorityField = (uint8)wan_priority_data << 5;	// just get most-significant 3 bits 
						
					*((uint16*)(&(mbuf->m_data[12]))) = htons(0x8100);
					*((uint16*)(&(mbuf->m_data[14]))) = htons(wan_vlan_id_data);
					*((uint8 *)(&(mbuf->m_data[14]))) &=0x0f ;
					*((uint8 *)(&(mbuf->m_data[14]))) |= htons(priorityField);
					//printk("in swNic_send , Data  gene, vlan: %d, priority: %d\n", pPkthdr->ph_vlanId, pPkthdr->ph_txPriority);
				}
			}
		}
		}
#endif

	pPkthdr->ph_vlanId=vid;

#ifdef _PKTHDR_CACHEABLE
	_dma_cache_wback_inv((unsigned long)pPkthdr, sizeof(struct pktHdr));
#ifdef TX_SCATTER
	{
	struct mBuf *mbuf2= pPkthdr->ph_mbuf;
	for (;;) {	
		_dma_cache_wback_inv((unsigned long)mbuf2, sizeof(struct mBuf));
		if (mbuf2->m_next == NULL)
			break;
		mbuf2 = mbuf2->m_next;
	}
	}
#else
	_dma_cache_wback_inv((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct mBuf));
#endif
#endif

#ifdef TX_SCATTER
	for (next_index=0; next_index<sk->list_num; next_index++)		
		_dma_cache_wback_inv((unsigned long)sk->list_buf[next_index].buf, sk->list_buf[next_index].len);
#else
	_dma_cache_wback_inv((unsigned long)output, len);
#endif
	/* Give descriptor to switch core */
	txPkthdrRing[0][ret] |= DESC_SWCORE_OWNED;

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
	{
		uint32 pkthdr2 = (uint32)txPkthdrRing[0][ret];
		if ((pkthdr2 & DESC_OWNED_BIT) == 0)
			diag_printf("_swNic_send: idx= %d, read back pkthdr= 0x%x.\n", ret, pkthdr2);
	}
#endif
	/* Set TXFD bit to start send */
	REG32(CPUICR) |= TXFD;

	//restore_flags(flags);
	restore_flags2(flags);
	return ret;
}

#if defined(CONFIG_RTK_CHECK_ETH_TX_HANG) && !defined(CONFIG_RTL_8196C_ESD_NEW)
static int _tx_hang_inc = 0;
static int32 saved_txPktDoneDescIndex=0;

// return 1: need reinit
int check_tx_desc_hang(void)
{
/*	if ((REG32(GISR) & LX0_BFRAME_IP) == LX0_BFRAME_IP) {
		panic_printk("===> GISR.LX0_BFRAME_IP = 1\n");
		//return 1;
	}  */
	
	if (txPktDoneDescIndex != currTxPkthdrDescIndex) {
		if (txPktDoneDescIndex == saved_txPktDoneDescIndex)
			_tx_hang_inc++;
		else {
			saved_txPktDoneDescIndex = txPktDoneDescIndex;
			_tx_hang_inc = 0;
		}
		if (_tx_hang_inc == 3) {
			RTL_swNic_txDone();
			if (txPktDoneDescIndex == saved_txPktDoneDescIndex) {
				// txPktDoneDescIndex is not changed after swNic_txDone() is called.
				_tx_hang_inc = 0;
				return 1;
			}
			else {
				saved_txPktDoneDescIndex = txPktDoneDescIndex;
				_tx_hang_inc = 0;
			}
		}
	}
	else
		_tx_hang_inc = 0;
	return 0;
}		
#endif

// refer to swNic_txDone()
int32 free_pending_tx_skb(void)
{
	struct pktHdr *pPkthdr;
	
	while (txPktDoneDescIndex != currTxPkthdrDescIndex) {		
		pPkthdr = (struct pktHdr *) ((int32) txPkthdrRing[0][txPktDoneDescIndex] 
                            & ~(DESC_OWNED_BIT | DESC_WRAP));
#ifdef ETH_NEW_FC
		if (pPkthdr && pPkthdr->ph_mbuf && pPkthdr->ph_mbuf->skb)	{
			tx_done_callback(pPkthdr->ph_mbuf->skb);
			pPkthdr->ph_mbuf->skb = NULL;
		}
#else
		if (pPkthdr->skb)	{
			tx_done_callback(pPkthdr->skb);
			pPkthdr->skb = NULL;
		}
#endif
		if (++txPktDoneDescIndex == txPkthdrRingCnt[0])
			txPktDoneDescIndex = 0;
	}

	return 0;	
}

int32 swNic_txDone(void)
{
	struct pktHdr * pPkthdr;
	int free_num;
	
	while (txPktDoneDescIndex != currTxPkthdrDescIndex) {		
	    if ( (*(volatile uint32 *)&txPkthdrRing[0][txPktDoneDescIndex] 
                    & DESC_OWNED_BIT) == DESC_RISC_OWNED ) {
                 	#ifdef CONFIG_RTL8196C_REVISION_B
			if (REG32(REVR) == RTL8196C_REVISION_A)
		            txPkthdrRing[0][txPktDoneDescIndex] =txPkthdrRing_BACKUP[0][txPktDoneDescIndex] ;
		      #endif
			#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
			pPkthdr = (struct pktHdr *) (txPkthdrRing_base[0] + (sizeof(struct pktHdr) * txPktDoneDescIndex));								
			#else
		    	pPkthdr = (struct pktHdr *) ((int32) txPkthdrRing[0][txPktDoneDescIndex] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));
			#endif
			
#ifdef ETH_NEW_FC
			if (pPkthdr->ph_mbuf->skb)	{
				tx_done_callback(pPkthdr->ph_mbuf->skb);
				pPkthdr->ph_mbuf->skb = NULL;
			}
#else
			if (pPkthdr->skb)	{
				tx_done_callback(pPkthdr->skb);
				pPkthdr->skb = NULL;
			}
#endif

#ifdef TX_SCATTER
			{
				struct mBuf *pmbuf, *pmbuf_next;
				pmbuf = pPkthdr->ph_mbuf->m_next; //keep 1st mbuf on the pkthdr, don't free it to mbuf_que
				while (pmbuf) {
					pmbuf_next = pmbuf->m_next;
					list_add_tail((struct list_head *)&pmbuf->list, &mbuf_que);
					pmbuf = pmbuf_next;
				}
			}
#endif

			pPkthdr->ph_flags &= ~(PKTHDR_HWLOOKUP|PKTHDR_BRIDGING);

#ifdef CONFIG_RTL865X_HW_PPTPL2TP
			pPkthdr->ph_srcExtPortNum = 0;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_flags &= ~CSUM_IP;
#endif

			if (++txPktDoneDescIndex == txPkthdrRingCnt[0])
				txPktDoneDescIndex = 0;
		}
		else
			break;
	}

	if (currTxPkthdrDescIndex >= txPktDoneDescIndex)
		free_num =  txPkthdrRingCnt[0] - currTxPkthdrDescIndex + txPktDoneDescIndex;
	else
		free_num = txPktDoneDescIndex - currTxPkthdrDescIndex - 1;
	return free_num;	
}


#ifdef  CONFIG_RTL865X_MODEL_TEST_FT2
int32 swNic_send_portmbr(void * output, uint32 len, uint32 portmbr)
{
    struct pktHdr * pPkthdr;
    uint8 pktbuf[2048];
    uint8* pktbuf_alligned = (uint8*) (( (uint32) pktbuf & 0xfffffffc) | 0xa0000000);

    /* Copy Packet Content */
    memcpy(pktbuf_alligned, output, len);

    ASSERT_CSP( ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED );

    /* Fetch packet header from Tx ring */
    pPkthdr = (struct pktHdr *) ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

    /* Pad small packets and add CRC */
    if ( len < 60 )
        pPkthdr->ph_len = 64;
    else
        pPkthdr->ph_len = len + 4;

    pPkthdr->ph_mbuf->m_len = pPkthdr->ph_len;
    pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;

    /* Set cluster pointer to buffer */
    pPkthdr->ph_mbuf->m_data = pktbuf_alligned;
    pPkthdr->ph_mbuf->m_extbuf = pktbuf_alligned;

    /* Set destination port */
    pPkthdr->ph_portlist = portmbr;

    /* Give descriptor to switch core */
    txPkthdrRing[0][currTxPkthdrDescIndex] |= DESC_SWCORE_OWNED;

    /* Set TXFD bit to start send */
    REG32(CPUICR) |= TXFD;
    
    /* Wait until packet is successfully sent */
#if 1    
    while ( (*(volatile uint32 *)&txPkthdrRing[0][currTxPkthdrDescIndex] 
                    & DESC_OWNED_BIT) == DESC_SWCORE_OWNED );
#endif    
    txPktCounter++;
    
    if ( ++currTxPkthdrDescIndex == txPkthdrRingCnt[0] )
        currTxPkthdrDescIndex = 0;

    return 0;
}
#endif


void swNic_freeRxBuf(void)
{
	int idx;

#ifdef ETH_NEW_FC
	struct mBuf * pMbuf;

	for (idx=0; idx<rxMbufRingCnt; idx++) {
//	    if (!((rxMbufRing[idx] & DESC_OWNED_BIT) == DESC_RISC_OWNED)) 
	    {
			pMbuf = (struct mBuf *) (rxMbufRing[idx] & ~(DESC_OWNED_BIT | DESC_WRAP));
			if (pMbuf && pMbuf->skb)
				free_rx_buf(pMbuf->skb);				
	    }
	}
#else
	struct pktHdr * pPkthdr;

	for (idx=0; idx<rxPkthdrRingCnt[0]; idx++) {
	    if (!((rxPkthdrRing[0][idx] & DESC_OWNED_BIT) == DESC_RISC_OWNED)) {
			pPkthdr = (struct pktHdr *) (rxPkthdrRing[0][idx] & 
                                            ~(DESC_OWNED_BIT | DESC_WRAP));    
			if (pPkthdr->skb)
				free_rx_buf(pPkthdr->skb);
	    }
    }
#endif
}

//#pragma ghs section text=default
/*************************************************************************
*   FUNCTION                                                              
*       swNic_init                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function initializes descriptors and data structures.
*                                                                         
*   INPUTS                                                                
*       userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC] :
*          Number of Rx pkthdr descriptors of each ring.
*       userNeedRxMbufRingCnt :
*          Number of Tx mbuf descriptors.
*       userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC] :
*          Number of Tx pkthdr descriptors of each ring.
*       clusterSize :
*          Size of cluster.
*                                                                         
*   OUTPUTS                                                               
*       Status.
*************************************************************************/

int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC],
                 uint32 clusterSize)
{
	uint32 i, j, k;
	static uint32 totalRxPkthdrRingCnt = 0, totalTxPkthdrRingCnt = 0;
	static struct pktHdr *pPkthdrList_start;
	static struct mBuf *pMbufList_start;		
	struct pktHdr *pPkthdrList;
	struct mBuf *pMbufList;
	//uint8 * pClusterList;
	struct pktHdr * pPkthdr;
	struct mBuf * pMbuf;

#if	defined(CONFIG_RTL_HARDWARE_NAT)
	/* init const array for rx pre-process	*/
	extPortMaskToPortNum[0] = 5;
	extPortMaskToPortNum[1] = 6;
	extPortMaskToPortNum[2] = 7;
	extPortMaskToPortNum[3] = 5;
	extPortMaskToPortNum[4] = 8;
	extPortMaskToPortNum[5] = 5;
	extPortMaskToPortNum[6] = 5;
	extPortMaskToPortNum[7] = 5;
#endif

	if (rxMbufRing == NULL) { // first time	
		size_of_cluster = clusterSize;

		/* Allocate Rx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_RXRING_MAX_PKTDESC; i++) {   
			rxPkthdrRingCnt[i] = userNeedRxPkthdrRingCnt[i];
			if (rxPkthdrRingCnt[i] == 0)
 				continue;

			rxPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(rxPkthdrRingCnt[i] * sizeof(uint32));
			ASSERT_CSP( (uint32) rxPkthdrRing[i] & 0x0fffffff );

			totalRxPkthdrRingCnt += rxPkthdrRingCnt[i];
		}
    
		if (totalRxPkthdrRingCnt == 0)
			return EINVAL;

		/* Allocate Tx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_TXRING_MAX_PKTDESC; i++) {    
			txPkthdrRingCnt[i] = userNeedTxPkthdrRingCnt[i];

			if (txPkthdrRingCnt[i] == 0)
				continue;

			txPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32));
                 	#ifdef CONFIG_RTL8196C_REVISION_B
			if (REG32(REVR) == RTL8196C_REVISION_A)
			    txPkthdrRing_BACKUP[i]=(uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32));
			#endif
			ASSERT_CSP( (uint32) txPkthdrRing[i] & 0x0fffffff );

			totalTxPkthdrRingCnt += txPkthdrRingCnt[i];
		}

		if (RTL865X_SWNIC_TXRING_MAX_PKTDESC == 1) {
			/* The minimum size of each ring is two descriptors. */
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
			txPkthdrRing[1] = (uint32 *) UNCACHED_MALLOC(2 * sizeof(uint32));		
			txPkthdrRing[2] = (uint32 *) UNCACHED_MALLOC(2 * sizeof(uint32));		
			txPkthdrRing[3] = (uint32 *) UNCACHED_MALLOC(2 * sizeof(uint32));		
#else			
			txPkthdrRing[1] = (uint32 *) UNCACHED_MALLOC(2 * sizeof(uint32));		
#endif
		}

		if (totalTxPkthdrRingCnt == 0)
			return EINVAL;

		/* Allocate MBuf descriptors of rings */
		rxMbufRingCnt = userNeedRxMbufRingCnt;

		if (userNeedRxMbufRingCnt == 0)
			return EINVAL;

		rxMbufRing = (uint32 *) UNCACHED_MALLOC(userNeedRxMbufRingCnt * sizeof(uint32));
		ASSERT_CSP( (uint32) rxMbufRing & 0x0fffffff );

#ifdef _PKTHDR_CACHEABLE
		/* Allocate pkthdr */
		pPkthdrList_start = (struct pktHdr *) kmalloc(
			(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt+1) * sizeof(struct pktHdr), GFP_KERNEL);
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		pPkthdrList_start = (struct pktHdr *)(((uint32) pPkthdrList_start + (cpu_dcache_line - 1))& ~(cpu_dcache_line - 1));

		memset(pPkthdrList_start, 0, (totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct pktHdr));
		_dma_cache_wback_inv((unsigned long)pPkthdrList_start, (totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct pktHdr));

		/* Allocate mbufs */
#ifdef TX_SCATTER
		pMbufList_start = (struct mBuf *) kmalloc(
			(rxMbufRingCnt + totalTxPkthdrRingCnt*2+1) * sizeof(struct mBuf), GFP_KERNEL);

		pMbufList_start = (struct mBuf *)(((uint32) pMbufList_start + (cpu_dcache_line - 1))& ~(cpu_dcache_line - 1));

		memset(pMbufList_start, 0, (rxMbufRingCnt + totalTxPkthdrRingCnt*2) * sizeof(struct mBuf));

		_dma_cache_wback_inv((unsigned long)pMbufList_start, (rxMbufRingCnt + totalTxPkthdrRingCnt*2) * sizeof(struct mBuf));
#else
		pMbufList_start = (struct mBuf *) kmalloc(
			(rxMbufRingCnt + totalTxPkthdrRingCnt+1) * sizeof(struct mBuf), GFP_KERNEL);

		pMbufList_start = (struct mBuf *)(((uint32) pMbufList_start + (cpu_dcache_line - 1))& ~(cpu_dcache_line - 1));

		_dma_cache_wback_inv((unsigned long)pMbufList_start, (rxMbufRingCnt + totalTxPkthdrRingCnt) * sizeof(struct mBuf));
#endif
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );
#else
		/* Allocate pkthdr */
		pPkthdrList_start = (struct pktHdr *) UNCACHED_MALLOC(
			(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct pktHdr));
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );
                    
		/* Allocate mbufs */
#ifdef TX_SCATTER
		pMbufList_start = (struct mBuf *) UNCACHED_MALLOC(
			(rxMbufRingCnt + totalTxPkthdrRingCnt*2) * sizeof(struct mBuf));
#else
		pMbufList_start = (struct mBuf *) UNCACHED_MALLOC(
			(rxMbufRingCnt + totalTxPkthdrRingCnt) * sizeof(struct mBuf));
#endif
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );
#endif

#if 0										
		 /* Allocate clusters */
		pClusterList = (uint8 *) UNCACHED_MALLOC(rxMbufRingCnt * size_of_cluster + 8 - 1);
		ASSERT_CSP( (uint32) pClusterList & 0x0fffffff );
		pClusterList = (uint8*)(((uint32) pClusterList + 8 - 1) & ~(8 - 1));
#endif
	}

	/* Initialize interrupt statistics counter */
	rxPktCounter = txPktCounter = 0;

    /* Initialize index of Tx pkthdr descriptor */
    currTxPkthdrDescIndex = 0;
    txPktDoneDescIndex=0;

	pPkthdrList = pPkthdrList_start;
	pMbufList = pMbufList_start;

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)  ||defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	for (i = 0; i < 2; i++)
		REG32(CPUTPDCR(i)) = 0;
#endif

#ifdef TX_SCATTER
	INIT_LIST_HEAD(&mbuf_que);
#endif

    /* Initialize Tx packet header descriptors */
    for (i = 0; i < RTL865X_SWNIC_TXRING_MAX_PKTDESC; i++)
    {
        for (j = 0; j < txPkthdrRingCnt[i]; j++)
        {
            /* Dequeue pkthdr and mbuf */
            pPkthdr = pPkthdrList++;
            pMbuf = pMbufList++;

            bzero((void *) pPkthdr, sizeof(struct pktHdr));
            bzero((void *) pMbuf, sizeof(struct mBuf));
#ifdef TX_SCATTER
            list_add_tail((struct list_head *)&pMbuf->list, &mbuf_que);
            pMbuf = pMbufList++;
            pPkthdr->ph_mbuf = pMbuf;
#else
            pPkthdr->ph_mbuf = pMbuf;
#endif
            pPkthdr->ph_len = 0;
            pPkthdr->ph_flags = PKTHDR_USED | PKT_OUTGOING;
            pPkthdr->ph_type = PKTHDR_ETHERNET;
            pPkthdr->ph_portlist = 0;

            pMbuf->m_next = NULL;
            pMbuf->m_pkthdr = pPkthdr;
            pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
            pMbuf->m_data = NULL;
            pMbuf->m_extbuf = NULL;
            pMbuf->m_extsize = 0;

            txPkthdrRing[i][j] = (int32) pPkthdr | DESC_RISC_OWNED;
               #ifdef CONFIG_RTL8196C_REVISION_B
		  if (REG32(REVR) == RTL8196C_REVISION_A)
	            txPkthdrRing_BACKUP[i][j] = (int32) pPkthdr | DESC_RISC_OWNED;
	        #endif		
        }
		
#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		txPkthdrRing_base[i] = txPkthdrRing[i][0];
#endif
        /* Set wrap bit of the last descriptor */
        if (txPkthdrRingCnt[i] != 0)
        	{
            txPkthdrRing[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
               #ifdef CONFIG_RTL8196C_REVISION_B
		  if (REG32(REVR) == RTL8196C_REVISION_A)
		        txPkthdrRing_BACKUP[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
	        #endif
		}
		
        /* Fill Tx packet header FDP */
        REG32(CPUTPDCR(i)) = (uint32) txPkthdrRing[i];
    }

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
        REG32(DMA_CR1) = (REG32(DMA_CR1) & (~0x0FFF0000)) | ((txPkthdrRingCnt[0]-1) << 16);
#endif		

	if (RTL865X_SWNIC_TXRING_MAX_PKTDESC == 1) {
		/* initialize Tx descriptor ring 1 */
		txPkthdrRing[1][0] = DESC_RISC_OWNED;
		txPkthdrRing[1][1] = (DESC_WRAP | DESC_RISC_OWNED);
		REG32(CPUTPDCR(1)) = (uint32) txPkthdrRing[1];

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
		/* initialize Tx descriptor ring 1 */
		txPkthdrRing[2][0] = DESC_RISC_OWNED;
		txPkthdrRing[2][1] = (DESC_WRAP | DESC_RISC_OWNED);
		REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];

		/* initialize Tx descriptor ring 1 */
		txPkthdrRing[3][0] = DESC_RISC_OWNED;
		txPkthdrRing[3][1] = (DESC_WRAP | DESC_RISC_OWNED);
		REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif		
	}

    /* Initialize index of current Rx pkthdr descriptor */
    currRxPkthdrDescIndex = 0;

    /* Initialize index of current Rx Mbuf descriptor */
    currRxMbufDescIndex = 0;

#ifdef DELAY_REFILL_ETH_RX_BUF
	rxDescReadyForHwIndex = 0;
#endif

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
	for (i = 0; i < 6; i++)
		REG32(CPURPDCR(i)) = 0;
#endif		

    /* Initialize Rx packet header descriptors */
    k = 0;

    for (i = 0; i < RTL865X_SWNIC_RXRING_MAX_PKTDESC; i++)
    {
        for (j = 0; j < rxPkthdrRingCnt[i]; j++)
        {
            /* Dequeue pkthdr and mbuf */
            pPkthdr = pPkthdrList++;
            pMbuf = pMbufList++;

            bzero((void *) pPkthdr, sizeof(struct pktHdr));
            bzero((void *) pMbuf, sizeof(struct mBuf));

            /* Setup pkthdr and mbuf */
            pPkthdr->ph_mbuf = pMbuf;
            pPkthdr->ph_len = 0;
            pPkthdr->ph_flags = PKTHDR_USED | PKT_INCOMING;
            pPkthdr->ph_type = PKTHDR_ETHERNET;
            pPkthdr->ph_portlist = 0;
            pMbuf->m_next = NULL;
            pMbuf->m_pkthdr = pPkthdr;
            pMbuf->m_len = 0;
            pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
            pMbuf->m_extsize = size_of_cluster;
#ifdef ETH_NEW_FC
#ifdef DELAY_REFILL_ETH_RX_BUF
		/* when rxDescReadyForHwIndex != 0, we stop calling alloc_rx_buf() function because no buf remained */
		if (rxDescReadyForHwIndex == 0) {
	            pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pMbuf->skb, size_of_cluster);
			if (pMbuf->m_data == NULL) {
				rxDescReadyForHwIndex = j;
			}
		}
#else		
            pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pMbuf->skb, size_of_cluster);
#endif

#else
            pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pPkthdr->skb, size_of_cluster);
#endif
            _dma_cache_wback_inv((unsigned long)pMbuf->m_extbuf, size_of_cluster); //michael
            
            /* Setup descriptors */
            rxPkthdrRing[i][j] = (int32) pPkthdr | DESC_SWCORE_OWNED;
            rxMbufRing[k++] = (int32) pMbuf | DESC_SWCORE_OWNED;
        }

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		rxPkthdrRing_base[i] = rxPkthdrRing[i][0] & ~DESC_OWNED_BIT;
#endif

        /* Set wrap bit of the last descriptor */
        if (rxPkthdrRingCnt[i] != 0)
            rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

        /* Fill Rx packet header FDP */
        REG32(CPURPDCR(i)) = (uint32) rxPkthdrRing[i];

#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) ||defined(CONFIG_RTL_8197F)
        REG32(DMA_CR2) = (REG32(DMA_CR2) & (~0x0FFFFFF0)) | ((rxMbufRingCnt-1) << 16) | ((rxPkthdrRingCnt[i]-1) << 4);
#endif		
    }

    rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;

    /* Fill Rx packet header FDP */
//    REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
//    REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
//    REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
//    REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
//    REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
//    REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

    REG32(CPURMDCR0) = (uint32) rxMbufRing;

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
	extern void 	refill_rx_skb(void);
	refill_rx_skb();
#endif

    //printkf("addr=%x, val=%x\r\n",(CPUIIMR),REG32(CPUIIMR));
    /* Enable runout interrupts */
    //REG32(CPUIIMR) |= RX_ERR_IE_ALL | TX_ERR_IE_ALL | PKTHDR_DESC_RUNOUT_IE_ALL;  //8651c
    //REG32(CPUIIMR) = 0xffffffff; //RX_DONE_IE_ALL;  //   0xffffffff;  //wei test irq
    
    //*(volatile unsigned int*)(0xb8010028)=0xffffffff; 
    //printkf("eth0 CPUIIMR status=%x\r\n", *(volatile unsigned int*)(0xb8010028));   //ISR 
       
    /* Enable Rx & Tx. Config bus burst size and mbuf size. */
    //REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_256WORDS | icr_mbufsize;
    //REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;    //8651c
    //REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES; //wei test irq
    //REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE;// | TX_DONE_IE_ALL; 

    //printkf("eth0 CPUIIMR status=%x\r\n", *(volatile unsigned int*)(0xb8010028));   //ISR
    
    return SUCCESS;
}


#ifdef FAT_CODE
/*************************************************************************
*   FUNCTION                                                              
*       swNic_resetDescriptors                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function resets descriptors.
*                                                                         
*   INPUTS                                                                
*       None.
*                                                                         
*   OUTPUTS                                                               
*       None.
*************************************************************************/
void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}
#endif//FAT_CODE


#ifdef CONFIG_RTK_VOIP_WAN_VLAN
void rtl865xC_vlan_init()
{
	wan_vlan_enable = 0;
	
	del_WAN_VLAN(wan_vlan_id_proto);		
	wan_vlan_id_proto = DEFAULT_WAN_VLAN_ID;
	
	del_WAN_VLAN(wan_vlan_id_data);
	wan_vlan_id_data  = DEFAULT_WAN_VLAN_ID;
	
	del_WAN_VLAN(wan_vlan_id_video);
	wan_vlan_id_video = DEFAULT_WAN_VLAN_ID;
		                	
	wan_priority_proto = DEFAULT_VOICE_PRIORITY; 
	wan_priority_data  = DEFAULT_DATA_PRIORITY; 
	wan_priority_video = DEFAULT_VIDEO_PRIORITY;
	
	wan_cfi_proto = 0;
	wan_cfi_data = 0;
	wan_cfi_video = 0;
}

void rtl865xC_wan_3_vlan(
			unsigned int id_proto,
			unsigned int priority_proto,
			unsigned int cfi_proto,
			unsigned int id_data,
			unsigned int priority_data,
			unsigned int cfi_data,
			unsigned int id_video,
			unsigned int priority_video,
			unsigned int cfi_video
			)
{
	wan_vlan_enable = 1;
	
	del_WAN_VLAN(wan_vlan_id_proto);
	del_WAN_VLAN(wan_vlan_id_data);
	del_WAN_VLAN(wan_vlan_id_video);
		
	wan_vlan_id_proto = id_proto;
	wan_priority_proto = priority_proto;
	wan_cfi_proto = cfi_proto;
	add_WAN_VLAN(wan_vlan_id_proto);

	wan_vlan_id_data = id_data;
	wan_priority_data = priority_data;
	wan_cfi_data = cfi_data;
	add_WAN_VLAN(wan_vlan_id_data);

	wan_vlan_id_video = id_video;
	wan_priority_video = priority_video;
	wan_cfi_video = cfi_video;
	add_WAN_VLAN(wan_vlan_id_video);
}
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)
inline int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid)
{
	struct pktHdr *m_pkthdr = (struct pktHdr *)pkt;
	uint32 srcPortNum;
       srcPortNum = m_pkthdr->ph_portlist;

	#if 0
	if (srcPortNum >= RTL8651_CPU_PORT)
	{
		if (m_pkthdr->ph_extPortList == 0)
		{
			/* No any destination ( extension port or CPU) : ASIC's BUG */
			return FAILED;
		}else if ((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList];
			#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			m_pkthdr->ph_portlist = srcPortNum;
			*vid = PKTHDR_EXTPORT_MAGIC;
			#else
			*vid = m_pkthdr->ph_vlanId;
			#endif
		}else
		{
			/* has CPU bit, pkt is original pkt from port 6~8 */
			srcPortNum = m_pkthdr->ph_srcExtPortNum + RTL8651_PORT_NUMBER - 1;
			m_pkthdr->ph_portlist = srcPortNum;
			#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			*vid = PKTHDR_EXTPORT_MAGIC2;
			#else
			*vid = m_pkthdr->ph_vlanId;
			#endif
		}
	}
       else
	#else
	if (srcPortNum < RTL8651_CPU_PORT)
	#endif
	{
		/* otherwise, pkt is rcvd from PHY */
		m_pkthdr->ph_srcExtPortNum = 0;
		*vid = m_pkthdr->ph_vlanId;
		if((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
		{	/* No CPU bit, only dest ext mbr port... */
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			if(m_pkthdr->ph_extPortList&&5!=extPortMaskToPortNum[m_pkthdr->ph_extPortList])
			{
				/* redefine src port number */
				srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList];
				#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
				m_pkthdr->ph_portlist = srcPortNum;
				*vid = PKTHDR_EXTPORT_MAGIC;
				#else
				*vid = m_pkthdr->ph_vlanId;
				#endif
			}
		}
	}		else {
		return FAILED;
	}

	return SUCCESS;
}
#endif

