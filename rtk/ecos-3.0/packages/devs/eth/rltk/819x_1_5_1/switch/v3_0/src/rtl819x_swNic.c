/*
 *	Abstract: provide NIC initialization, reception	and	transmission
 *			  functions	for	new	descriptor architecture.
 *
 *	Author:
 *		Joey Lin, <joey.lin@realtek.com>
 *
 *	Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *	This program is	free software; you can redistribute	it and/or
 *	modify it under	the	terms of the GNU General Public	License
 *	as published by	the	Free Software Foundation; either version
 *	2 of the License, or (at your option) any later	version.
 */

#if	0
TODO: refer	to /linux-3.10/drivers/net/ethernet/broadcom/bcm63xx_enet.c
			   /linux-3.10/drivers/net/ethernet/realtek/r8169.c
	  and follow them to use those functions

rtl_open()
{
	/*
	 * Rx and Tx descriptors needs 256 bytes alignment.
	 * dma_alloc_coherent provides more.
	 */
	tp->RxDescArray	= dma_alloc_coherent(&pdev->dev, R8169_RX_RING_BYTES,
						 &tp->RxPhyAddr, GFP_KERNEL);
	.....
	smp_mb();
	.....
	dma_free_coherent(&pdev->dev, R8169_RX_RING_BYTES, tp->RxDescArray,
			  tp->RxPhyAddr);
}

static struct sk_buff *rtl8169_alloc_rx_data(struct	rtl8169_private	*tp,
						 struct	RxDesc *desc)
{
	.....
	mapping	= dma_map_single(d,	rtl8169_align(data), rx_buf_sz,
				 DMA_FROM_DEVICE);
	.....
}

static netdev_tx_t rtl8169_start_xmit(struct sk_buff *skb,
					  struct net_device	*dev)
{
	.....
	mapping	= dma_map_single(d,	skb->data, len,	DMA_TO_DEVICE);
	.....
	wmb();

	wmb();

	mmiowb();

		smp_wmb();

		smp_mb();

	.....
}
#endif

#ifdef __ECOS
#include <pkgconf/system.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include "rtl_types.h"
#include "rtl_errno.h"
#include "asicregs.h"
#include "swNic_poll.h"
#include "rtl865xC_tblAsicDrv.h"
#include "rtl819x_swNic.h"
#else
#include <linux/kconfig.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <uapi/linux/if_ether.h>
#include <uapi/linux/in.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <asm/cpu-features.h>
#include "common/rtl_errno.h"
#include "AsicDriver/asicRegs.h"
#include <net/rtl/rtl865x_netif.h>
#include "rtl865xc_swNic.h"
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "rtl819x_swNic.h"
#endif
//#include "m24kctrl.h" //added for romperf testing	

#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
extern unsigned	int	rx_noBuffer_cnt;
#endif

#ifdef __ECOS
extern void rtl_dev_kfree_skb_any(struct sk_buff *skb);
#endif

#define PKTHDR_BRIDGING      				0x0040     /* when PKTHDR_HWLOOKUP is on. 1: Hardware assist to do L2 bridging only, 0:hardware assist to do NAPT*/
#define PKTHDR_HWLOOKUP      			0x0020	/* valid when ph_extPortList!=0. 1:Hardware table lookup assistance*/
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
struct ring_info {
	unsigned int		skb;
//	unsigned int		frag;	// no used now
};

static struct ring_info	*rx_skb[NEW_NIC_MAX_RX_DESC_RING];
static struct ring_info	*tx_skb[NEW_NIC_MAX_TX_DESC_RING];

static int32		 New_rxDescRingCnt[NEW_NIC_MAX_RX_DESC_RING];
static DMA_RX_DESC			*New_rxDescRing[NEW_NIC_MAX_RX_DESC_RING] =	{NULL};
static int32		 New_currRxPkthdrDescIndex[NEW_NIC_MAX_RX_DESC_RING];

static uint32	 New_txDescRingCnt[NEW_NIC_MAX_TX_DESC_RING];
static DMA_TX_DESC			*New_txDescRing[NEW_NIC_MAX_TX_DESC_RING];

static uint32 New_currTxPkthdrDescIndex[NEW_NIC_MAX_TX_DESC_RING];
static uint32 New_txPktDoneDescIndex[NEW_NIC_MAX_TX_DESC_RING];

static uint32 New_txDescRing_base[RTL865X_SWNIC_TXRING_HW_PKTDESC];
static uint32 New_rxDescRing_base[RTL865X_SWNIC_RXRING_HW_PKTDESC];

static uint32 TxCDP_reg[RTL865X_SWNIC_TXRING_HW_PKTDESC] =
	{ CPUTPDCR0, CPUTPDCR1,	CPUTPDCR2, CPUTPDCR3};

#define	NEXT_IDX(N,RING_SIZE)	( ((N+1) ==	RING_SIZE) ? 0 : (N+1) )

#define _DESC_CACHE_ACCESS_RX			1

// must	define USE_SWITCH_TX_CDP when use _DESC_CACHE_ACCESS_TX
/*
  when _DESC_CACHE_ACCESS_TX and USE_SWITCH_TX_CDP are both defined,
  in New_swNic_txDone(), for example,
  it will happen: 
    New_currTxPkthdrDescIndex[0] = 16
    New_txPktDoneDescIndex[0] = 12
    hw_cdp_idx = 17
  after New_swNic_txDone() done, will be:
    New_currTxPkthdrDescIndex[0] = 16
    New_txPktDoneDescIndex[0] = 17
    hw_cdp_idx = 17
  ==> Tx will STOP.
 */
//#define	_DESC_CACHE_ACCESS_TX		1

/* since _DESC_CACHE_ACCESS_TX can not used, we use a local TxDesc to store
   the setting, then copy the local version to physical desc memory once.
 */
#define _LOCAL_TX_DESC	1
static struct tx_desc local_txd;

#define	USE_SWITCH_TX_CDP	1 // use Switch	Tx Current Descriptor Pointer, instead of OWN bit.

/*
  in my	test in	FPGA, RxRing0 desc number is only 32,
  I	have connect 1 LAN PC and 1	WAN	PC,	then boot up kernel,
  after	the	ethernet interface up, and New_swNic_receive() be executed first time,
  the sw_curr_idx is 0 and hw_desc_idx is 0,
  but HW is	received 32	pkt	and	desc ran out.

  MUST figure out a	solution to	fix	above situation,
  then USE_SWITCH_RX_CDP flag can be used.
 */
#define USE_SWITCH_RX_CDP	1 // use Switch	Rx CDP

#if defined(CONFIG_RTL_VLAN_SUPPORT)
extern int rtl_vlan_support_enable;
#endif

void New_dumpTxRing(void)
{
	uint32 i,j;
	struct sk_buff *skb;
	uint32 *p;

	diag_printf("=========================	dump tx	ring =========================\n");

	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++)
	{
		diag_printf("====== ring %d ======\n",i);
		diag_printf("txDoneIdx[%d]: %d, txCurrIdx[%d]:	%d\n",i,
			New_txPktDoneDescIndex[i],i,New_currTxPkthdrDescIndex[i]);
		diag_printf("HW CDP: 0x%x,	HW idx (calculated): %d\n",
			REG32(TxCDP_reg[i]), (REG32(TxCDP_reg[i]) -	New_txDescRing_base[i])	/ sizeof(struct	dma_tx_desc));

		for	(j = 0;	j <	New_txDescRingCnt[i]; j++)
		{
			diag_printf("tx_desc[%d][%d]: 0x%x,",i,j,(uint32)(&New_txDescRing[i][j]) );

			p =	(uint32	*)(&New_txDescRing[i][j]);
			p =	(uint32	*)(((uint32)p) | UNCACHE_MASK);
			diag_printf("	%s owned\n", ((*p) & DESC_OWNED_BIT) ? "SWCORE"	: "CPU"	);

			diag_printf("	 6DW: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
					(uint32)(*p), (uint32)*(p+1),
					(uint32)*(p+2),	(uint32)*(p+3),
					(uint32)*(p+4),	(uint32)*(p+5) );
		}
		diag_printf("\n");
	}

	diag_printf("=========================	dump tx	skb	=========================\n");
	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++)
	{
		diag_printf("====== for ring %d ======\n",i);
		for	(j = 0;	j <	New_txDescRingCnt[i]; j++)
		{
			skb	= (struct sk_buff *)tx_skb[i][j].skb;
			if (skb	== NULL)
				diag_printf("[%d][%d]: tx_skb: %x, skb: NULL\n",i,j,
					(uint32)(&tx_skb[i][j])	);
			else
				diag_printf("[%d][%d]: tx_skb: %x, skb: %x, skb->data: %x\n",i,j,
					(uint32)(&tx_skb[i][j]), (uint32)tx_skb[i][j].skb,
					(uint32)skb->data);
		}
		diag_printf("\n");
	}
}

void New_dumpRxRing(void)
{
	uint32 i,j;
	struct sk_buff *skb;
	uint32 *p;

	diag_printf("=========================	dump rx	ring =========================\n");

	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		diag_printf("====== ring %d ======\n",i);
		diag_printf("currRxIdx[%d]: %d\n",i,New_currRxPkthdrDescIndex[i]);

		diag_printf("HW CDP: 0x%x,	HW idx (calculated): %d\n",
			REG32(CPURPDCR(i)),	(REG32(CPURPDCR(i))	- New_rxDescRing_base[i]) /	sizeof(struct dma_rx_desc));

		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			diag_printf("rx_desc[%d][%d]: 0x%x,",i,j,(uint32)(&New_rxDescRing[i][j]) );

			p =	(uint32	*)(&New_rxDescRing[i][j]);
			p =	(uint32	*)(((uint32)p) | UNCACHE_MASK);
			diag_printf("	%s owned\n", ((*p)&DESC_OWNED_BIT) ? "SWCORE" :	"CPU" );

			diag_printf("	 6DW: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
					(uint32)(*p), (uint32)*(p+1),
					(uint32)*(p+2),	(uint32)*(p+3),
					(uint32)*(p+4),	(uint32)*(p+5) );
		}
		diag_printf("\n");
	}

	diag_printf("=========================	dump rx	skb	=========================\n");
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		diag_printf("====== for ring %d ======\n",i);
		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			skb	= (struct sk_buff *)rx_skb[i][j].skb;
			if (skb	== NULL)
				diag_printf("[%d][%d]: rx_skb: %x, skb: NULL\n",i,j,
					(uint32)(&rx_skb[i][j])	);
			else
				diag_printf("[%d][%d]: rx_skb: %x, skb: %x, skb->data: %x\n",i,j,
						(uint32)(&rx_skb[i][j]), (uint32)rx_skb[i][j].skb,
						(uint32)skb->data);
		}
		diag_printf("\n");
	}
}

#if defined(__ECOS)&&defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
int can_send(void)
{
	int32 next_index;
	
	if ((New_currTxPkthdrDescIndex[0]+1) == New_txDescRingCnt[0])
		next_index = 0;
	else
		next_index = New_currTxPkthdrDescIndex[0]+1;

	if (next_index == New_txPktDoneDescIndex[0]) {
		RTL_swNic_txDone();
		if (next_index == New_txPktDoneDescIndex[0]) 	
			return 0;
	}

	if (New_txPktDoneDescIndex[0] > next_index)
		return (New_txPktDoneDescIndex[0] - next_index);
	else
		return (New_txDescRingCnt[0] - next_index + New_txPktDoneDescIndex[0]);
}
#endif

int32 New_swNic_init(uint32	userNeedRxPkthdrRingCnt[NEW_NIC_MAX_RX_DESC_RING],
				 uint32	userNeedTxPkthdrRingCnt[NEW_NIC_MAX_TX_DESC_RING],
				 uint32	clusterSize)
{
	uint32 i, j;
	static uint32 totalRxPkthdrRingCnt = 0,	totalTxPkthdrRingCnt = 0;
	unsigned long flags=0;
	int	ret	= SUCCESS;
	int	_cpu_dcache_line = cpu_dcache_line; // cpu_dcache_line_size(); //in \arch\mips\include\asm\cpu-features.h

	SMP_LOCK_ETH_RECV(flags);

	/* 1. Allocate Rx descriptor rings */
	if (New_rxDescRing[0] == NULL) {

	size_of_cluster = clusterSize;
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++) {
		New_rxDescRingCnt[i] = userNeedRxPkthdrRingCnt[i];
		if (New_rxDescRingCnt[i] ==	0)
		{
			New_rxDescRing[i] =	NULL;
			continue;
		}

		New_rxDescRing[i] =	(struct	dma_rx_desc	*) kmalloc((New_rxDescRingCnt[i]) *	sizeof(struct dma_rx_desc)+(2*_cpu_dcache_line), GFP_KERNEL);
		ASSERT_CSP(	(uint32) New_rxDescRing[i] & 0x0fffffff	);
		New_rxDescRing[i] =	(struct	dma_rx_desc	*)(((uint32) New_rxDescRing[i] + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
		memset(New_rxDescRing[i],0,New_rxDescRingCnt[i]	* sizeof(struct	dma_rx_desc));
		totalRxPkthdrRingCnt +=	New_rxDescRingCnt[i];

		if(New_rxDescRingCnt[i]){

			//rx_skb[i]=(struct	ring_info *)UNCACHED_MALLOC(sizeof(struct ring_info)*New_rxDescRingCnt[i]);
			rx_skb[i]=(struct ring_info	*)kmalloc(sizeof(struct ring_info)*New_rxDescRingCnt[i], GFP_KERNEL);

			if (!rx_skb[i])
			{
				goto err_out;
			}
			memset(rx_skb[i],0,New_rxDescRingCnt[i]	* sizeof(struct	ring_info));
		}
	}

	if (totalRxPkthdrRingCnt ==	0) {
		goto err_out;
	}

	/* 2. Allocate Tx descriptor rings */
	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++) {
		New_txDescRingCnt[i] = userNeedTxPkthdrRingCnt[i];

		if (New_txDescRingCnt[i] ==	0)
		{
			New_txDescRing[i] =	NULL;
			continue;
		}

		New_txDescRing[i] =	(struct	dma_tx_desc	*)kmalloc((New_txDescRingCnt[i]) * sizeof(struct dma_tx_desc)+ (_cpu_dcache_line*2), GFP_KERNEL);
		ASSERT_CSP(	(uint32) New_txDescRing[i] & 0x0fffffff	);
		New_txDescRing[i] =	(struct	dma_tx_desc	*)(((uint32) New_txDescRing[i] + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
		memset(New_txDescRing[i],0,New_txDescRingCnt[i]	* sizeof(struct	dma_tx_desc));
		totalTxPkthdrRingCnt +=	New_txDescRingCnt[i];

		if(New_txDescRingCnt[i]){

			tx_skb[i]=(struct ring_info	*)kmalloc(sizeof(struct ring_info)*New_txDescRingCnt[i], GFP_KERNEL);
			if (!tx_skb[i])
			{
				goto err_out;
			}
			memset(tx_skb[i],0,New_txDescRingCnt[i]	* sizeof(struct	ring_info));
		}
	}

	if (totalTxPkthdrRingCnt ==	0) {
		goto err_out;
	}
	}

	/* Initialize index	of Tx pkthdr descriptor	*/
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
		New_txDescRing[i][New_txDescRingCnt[i] - 1].opts1 |= DESC_WRAP;
		New_currTxPkthdrDescIndex[i] = 0;
		New_txPktDoneDescIndex[i]=0;
		New_txDescRing_base[i] = ((uint32) New_txDescRing[i]) |	UNCACHE_MASK;
		REG32(TxCDP_reg[i])	= New_txDescRing_base[i];

		if (New_txDescRing[i] != NULL)
			_dma_cache_wback_inv((unsigned long)New_txDescRing[i],(unsigned	long)(New_txDescRingCnt[i] * sizeof(struct dma_tx_desc)));
	}

	/* Maximum TX packet header 0 descriptor entries.
	   the register only provide 16-bit for each tx ring,
	   this way (set TX_RING0_TAIL_AWARE bit) CAN NOT used when tx desc number is larger than 2730.
	 */
	REG32(DMA_CR1) = (New_txDescRingCnt[0]-1) * (sizeof(struct tx_desc)); 
	REG32(DMA_CR4) = TX_RING0_TAIL_AWARE;
	
	/* Initialize Rx packet	header descriptors */
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			void *skb;

			New_rxDescRing[i][j].addr =
				(uint32)alloc_rx_buf((void **)&skb,	clusterSize);

			// todo: error check, if addr/skb =	NULL, ....
			rx_skb[i][j].skb = (uint32)skb;

			if (j == (New_rxDescRingCnt[i] - 1))
				New_rxDescRing[i][j].opts1 = (DESC_SWCORE_OWNED	| DESC_WRAP	|(clusterSize<<RD_M_EXTSIZE_OFFSET));
			else
				New_rxDescRing[i][j].opts1 = (DESC_SWCORE_OWNED	|(clusterSize<<RD_M_EXTSIZE_OFFSET)	);
		}
		/* Initialize index	of current Rx pkthdr descriptor	*/
		New_currRxPkthdrDescIndex[i] = 0;
		New_rxDescRing_base[i] = ((uint32) New_rxDescRing[i]) |	UNCACHE_MASK;
		REG32(CPURPDCR(i)) = New_rxDescRing_base[i];

		if (New_rxDescRing[i] != NULL)
			_dma_cache_wback_inv((unsigned long)New_rxDescRing[i],(unsigned	long)(New_rxDescRingCnt[i] * sizeof(struct dma_rx_desc)));
	}

	SMP_UNLOCK_ETH_RECV(flags);
	return ret;

err_out:
	SMP_UNLOCK_ETH_RECV(flags);
	return (-EINVAL);
}

#define	SET_OWN_BIT(desc) \
	desc->rx_own = DESC_SWCORE_OWNED;

#define	INC_IDX(ring, idx) \
	idx	= NEXT_IDX(idx,New_rxDescRingCnt[ring]);	\
	New_currRxPkthdrDescIndex[ring]	= idx;

#define	REUSE_DESC() \
	SET_OWN_BIT(desc); \
	INC_IDX(ring_idx, rx_idx); 

int32 New_swNic_receive(void** input, uint32* pLen, unsigned int *vid, unsigned int *pid)
{
	uint32 rx_idx, ring_idx;
	struct sk_buff *r_skb;
	struct rx_desc *desc;
	unsigned char *buf;
	void *skb;

	#ifdef USE_SWITCH_RX_CDP
	uint32 cdp_value;
	int	hw_idx;
	#endif

	#if	!defined(CONFIG_SMP)
	unsigned long flags	= 0;
	SMP_LOCK_ETH_RECV(flags);
	#endif

	for(ring_idx=0;ring_idx<1;ring_idx++)
	{
get_next:
		rx_idx = New_currRxPkthdrDescIndex[ring_idx];

		#ifdef USE_SWITCH_RX_CDP
		cdp_value =	REG32(CPURPDCR(ring_idx));
		hw_idx = (cdp_value	- New_rxDescRing_base[ring_idx]) / sizeof(struct dma_rx_desc);
		if (rx_idx == hw_idx) { // no incoming pkt or Rx desc runout
			// check runout case
			desc = (struct rx_desc *)&New_rxDescRing[ring_idx][rx_idx];
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			if (desc->rx_own)	// SWCORE owned
			// end of "check runout case"
				
				continue; // go for next rx ring
		}
		#endif

		desc = (struct rx_desc *)&New_rxDescRing[ring_idx][rx_idx];

		// NEEDED, do not removed. when	previous interrupt_dsr_rx()	is finished,
		//		the	New_rxDescRing[ring_idx][rx_idx] has been read,	and	found "no more packet",
		//		so its address is cached.
		//		if _dma_cache_inv()	removed, and one packet	come in, read the desc and it own bit
		//		is still SWCORE	owned.
		// TODO: need to test, remove this one,	and	add	_dma_cache_inv to the next IF (own bit is SWCORE owned)

		// todo: in	FPGA, when rx_desc use cacheable and do	_dma_cache_inv,	the	rx ring	will abnormal
		#ifdef _DESC_CACHE_ACCESS_RX
		_dma_cache_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
		#else
		desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
		#endif

		#ifndef	USE_SWITCH_RX_CDP
		//1.check desc's own bit
		if (desc->rx_own) //1: SWCORE owned
		{
			continue; // go	for	next rx	ring
		}
		#endif

		#if 0//def__ECOS
		if (desc->rx_len == 0) {
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}
		#endif

		// if checksum failed, reuse this desc, except the ipv6 ready logo case.
		if (!(desc->rx_l3csok) || !(desc->rx_l4csok))
		{
			#ifdef CONFIG_RTL_IPV6READYLOGO
			if (!( !(desc->rx_l4csok) && 
				   (desc->rx_ipv6) && (desc->rx_reason==0x60)))
			#endif	
			{
				#ifdef _DESC_CACHE_ACCESS_RX
				desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
				#endif
				REUSE_DESC();
				goto get_next;
			}
		}

		// porting from swNic_receive(), need or not ??
		#if 0 //defined(CONFIG_RTL_HARDWARE_NAT)
		if (desc->rx_spa >= RTL8651_CPU_PORT) {
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}
		#endif
		
		//2.rx skb
		r_skb =	(struct	sk_buff	*)(rx_skb[ring_idx][rx_idx].skb);
		if (r_skb == NULL) {	// should not happen
			// todo: may call alloc_rx_buf() to	get	a new skb,....
			diag_printf("ETH_ERR: skb_array[%d][%d] is NULL\n",ring_idx,rx_idx);
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}

		//3.alloc new rx skb
		buf	= alloc_rx_buf(&skb, size_of_cluster);
		if (buf) {
			//update to	info struct
			//info->priority = ring_idx; //	todo
			*vid =	desc->rx_dvlanid;
			*pid =	desc->rx_spa;
			//info->rxPri	= desc->rx_dpri;
			r_skb->len = desc->rx_len - 4;
			*input	= (void *)r_skb;
			*pLen =	r_skb->len;
			// todo, update	the	other field: priority, ...

			/* after enable	Rx scatter gather feature (0xb80100a4 bit5 cf_rx_gather	= 1),
			 * the mdata of	second/third/... pkthdr	will be	updated	to 4-byte alignment	by hardware.
			 */
			r_skb->data	= (unsigned	char *)(desc->mdata);

			// should remove this note ??
			/* 2015-11-13, since 8198C has a WAN to WLAN throughput low issue (branch 3.4 rev. 34320).
			 * we do the _dma_cache_inv after allocate a new skb, instead of after packet received.
			 * David's suggestion: in 8197F and New descriptor architecture, also apply this change.
			 * so _dma_cache_inv is move down.
			 */
			//_dma_cache_inv((unsigned long)r_skb->data, r_skb->len);

			//4.hook
			rx_skb[ring_idx][rx_idx].skb = (uint32)skb;

			#ifdef _DESC_CACHE_ACCESS_RX
			//desc->addr = (uint32)buf;
			//_dma_cache_wback_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			desc->mdata = (uint32)buf;
			#else
			desc->mdata = (uint32)buf;
			#endif

			// 2015-11-13, see the comment above.
			_dma_cache_inv((unsigned long)((struct sk_buff *)skb)->data, 1536);

			//5.set	own	bit, move down
			SET_OWN_BIT(desc);

			//6. increase rx index
			INC_IDX(ring_idx, rx_idx);

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return 0;
		}
		else {
			// just	leave this packet in rx	desc
			// do not change own_bit and rx_index
			#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
			rx_noBuffer_cnt++;
			#endif

			#if	defined(CONFIG_RTL_ETH_PRIV_SKB)
			//printk("ETH_ERR: no private skb buffer\n");
			#else
			//printk("ETH_ERR: kernel allocate skb failed\n");
			#endif

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return -1;
		}
	}

	#if	!defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
	#endif
	return (-1);
}

#if 0
static inline void fill_txd_misc(struct tx_desc *txd, rtl_nicTx_info *nicTx, struct sk_buff *skb)
{
	// extract from	_swNic_send, purpose: keep a function shortly and easy to read
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	txd->tx_dpri = nicTx->priority;
	#endif

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->tx_vlantag = nicTx->tagport;
	else
		txd->tx_vlantag = 0;
	#endif

	#ifdef CONFIG_RTL_VLAN_8021Q
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG)
			if(txd->tx_dp == RTL_WANPORT_MASK)
				txd->tx_vlantag = txd->tx_dvlanid!=RTL_WANVLANID ? RTL_WANPORT_MASK : 0; 
			else
				txd->tx_vlantag = RTL_WANPORT_MASK & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		#else
			txd->tx_vlantag = 0x3f & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		#endif
	}
	else
		txd->tx_vlantag = 0;

	#if defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	if(txd->tx_dp == RTL_WANPORT_MASK && (txd->tx_vlantag & RTL_WANPORT_MASK))
	{
		if(skb->srcVlanPriority>=0 && skb->srcVlanPriority<=7)
			txd->tx_dpri = skb->srcVlanPriority;
	}
  	#endif

	#elif defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#ifdef	CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
			#ifdef CONFIG_RTL_8367R_SUPPORT
			rtk_get_real_nicTxVid(skb, &nicTx->vid);
			txd->tx_dvlanid = nicTx->vid;
			#endif
			txd->tx_vlantag = (0x3f	& rtk_getTagPortmaskByVid(nicTx->vid,(void *)skb->data));
		#else
			txd->tx_vlantag = auto_set_tag_portmask;
		#endif
	}
	else
		txd->tx_vlantag = 0;
	#endif
}

static inline void fill_txd_tso(struct tx_desc	*txd, char *data)
{
	/* TODO:
		check dev->features's NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT
		bits(set in	re865x_probe), if true then	set	these 2	bits of	the	Tx pktHdr,
		NOT	always set.

		due	to we enable NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT bits
		of dev->features in	re865x_probe(),	so we must enable hardware L3/L4 checksum offload feature.
	 */
	uint16 type	= htons( *(uint16 *)(data+12) );

	txd->tx_l4cs = 1;
	txd->tx_l3cs = 1;

	// TODO: the type is listed	?? vi/li/pi/...	need to	be checked
	if (type ==	ETH_P_IPV6)	{
		txd->tx_type = PKTHDR_IPV6;
		txd->tx_ipv6 = 1;
	}
	else if	(type == ETH_P_IP) {
		struct iphdr *iph;

		iph	= (struct iphdr	*)(data+14);

		if (iph->protocol == IPPROTO_TCP)
			txd->tx_type = PKTHDR_TCP;
		else if	(iph->protocol == IPPROTO_UDP)
			txd->tx_type = PKTHDR_UDP;
		else if	(iph->protocol == IPPROTO_ICMP)
			txd->tx_type = PKTHDR_ICMP;
		else if	(iph->protocol == IPPROTO_IGMP)
			txd->tx_type = PKTHDR_IGMP;
		else
			txd->tx_type = PKTHDR_IP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;
	}
	else {
		txd->tx_type = PKTHDR_ETHERNET;
	}
}
#endif
static inline uint32 *dw_copy(uint32 *dest, uint32 *src, int cnt)
{
	int i;
	uint32 *org_dest;
	dest = (uint32 *)(((uint32)dest)| UNCACHE_MASK);
	org_dest = dest;
	for (i=0; i<cnt; i++)
		*dest++ = *src++;
	return (org_dest);
}

/*************************************************************************
*	FUNCTION
*		_New_swNic_send
*
*	DESCRIPTION
*		This function writes one packet	to tx descriptors, and waits until
*		the	packet is successfully sent.
*
*	INPUTS
*		None
*
*	OUTPUTS
*		None
*************************************************************************/

int32 _New_swNic_send(void *skb, void * output, uint32 len,unsigned int vid, unsigned int pid)
{
	uint32 len_tmp;
	uint32 tx_idx;
	struct tx_desc *txd;
	uint32 next_index;
//	startCP3Ctrl(0);	//added for romperf testing	
	tx_idx = New_currTxPkthdrDescIndex[0];

	next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[0]);

	if (next_index == New_txPktDoneDescIndex[0])	{
		/* TX ring full	*/
//		stopCP3Ctrl(1, 0);	//added for romperf testing	
		return -1;
	}

	#ifdef _LOCAL_TX_DESC
	txd	= &local_txd;
	#else
	txd	= (struct tx_desc *)&New_txDescRing[0][tx_idx];
	// txd should NOT be NULL, otherwise ....
	
	#ifndef	_DESC_CACHE_ACCESS_TX
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif

	memset((void *)txd, 0, sizeof(struct tx_desc));
	// store the skb pointer for New_swNic_txDone later
	tx_skb[0][tx_idx].skb = (uint32)skb;

	len_tmp = len;
	/* Pad small packets and hardware will add CRC */
	if ( len < 60 )
		len	= 64;
	else
		len	+= 4;

#if 0//def TX_SCATTER
	if (skb->list_num == 1) {
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
	txd->mdata  = ((uint32)output);
#endif

	//txd->mdata  = ((uint32)output);

	// do not need to set tail bit after we use DMA_CR1/DMA_CR4 register
	//if (tx_idx == (New_txDescRingCnt[nicTx->txIdx] - 1))
	//	txd->tx_eor = 1;
	txd->tx_ls = 1;
	txd->tx_fs = 1;
	txd->tx_ph_len = len;
	txd->tx_mlen = len;
	txd->tx_dvlanid = vid;
	txd->tx_dp = pid;

	txd->tx_extspa = 0;
	//diag_printf("pPkthdr->ph_portlist:%x,sk->cb[0]:%x,[%s]:[%d].\n",pPkthdr->ph_portlist,sk->cb[0],__FUNCTION__,__LINE__);
// -- Modify for IGMP snooping for Ethernet port ------------
	if (((struct sk_buff *)skb)->data[0]&0x01) {
		{
			txd->tx_dp = pid;
		}		
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
			//txd->tx_dp = RTL8651_CPU_PORT;		/* must be set 0x7 */
			/*
			     pPkthdr->ph_srcExtPortNum = 1, vlan table's bit 6 must be set
			     pPkthdr->ph_srcExtPortNum = 2, vlan table's bit 7 must be set
			     pPkthdr->ph_srcExtPortNum = 3, vlan table's bit 8 must be set
			 */
			//txd->tx_flags_ |= (PKTHDR_HWLOOKUP|PKTHDR_BRIDGING);

			txd->tx_hwlkup = 1;
			txd->tx_bridge = 1;
			txd->tx_extspa = PKTHDR_EXTPORT_LIST_P1;
			//txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;
		}
	}

	#ifdef CONFIG_RTL_TSO
	/* due to we enable NETIF_F_HW_CSUM in dev->features,
	   we need to enable HW Tx CSUM if ip_summed = CHECKSUM_PARTIAL.
	   TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
	   		 how can it work?
	 */
	if (((struct sk_buff *)skb)->ip_summed == CHECKSUM_PARTIAL) {
		nicTx->flags |= PKTHDR_HWLOOKUP;
		
		txd->tx_l4cs = 1;
		txd->tx_l3cs = 1;		
	}
	#endif
	
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	//fill_txd_misc(txd, nicTx, (struct sk_buff *)skb);
	#endif

#if 0//def TX_SCATTER
	for (next_index=0; next_index<sk->list_num; next_index++)		
		_dma_cache_wback_inv((unsigned long)sk->list_buf[next_index].buf, sk->list_buf[next_index].len);
#else
	_dma_cache_wback_inv((unsigned long)output, len_tmp);
#endif

	#ifdef _LOCAL_TX_DESC
	txd	= (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[0][tx_idx],
			(uint32 *)txd, sizeof(struct tx_desc) / 4);
	#else
	/* note: cache_wback of	output data	has	been done in caller	*/
	#ifdef _DESC_CACHE_ACCESS_TX
	_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif
	
	txd->tx_own = 1; //	set	own	bit	after all done.

	New_currTxPkthdrDescIndex[0]	= next_index;

	/* Set TXFD	bit	to start transmission */
	REG32(CPUICR) |= TXFD;
//	stopCP3Ctrl(1, 0);	//added for romperf testing	
	return 0;
}

#ifdef CONFIG_RTL_TSO
int32 _New_swNic_send_tso_sg(struct	sk_buff	*skb, void * output, uint32	len,rtl_nicTx_info *nicTx)
{
	uint32 tx_idx =	0, ret = SUCCESS, eor =	0;
	DMA_TX_DESC	*txd = NULL, *first_txd=NULL;
	uint32 next_index;
	uint32 cur_frag;
	uint32 first_len;
	uint32 skb_gso_size	= skb_shinfo(skb)->gso_size;

	first_len =	skb->len - skb->data_len;

	if(first_len)
	{
		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= (DMA_TX_DESC *)&local_txd;
		memset((void *)txd, 0, sizeof(DMA_TX_DESC));
		#else
		// todo: error check, if txd = NULL....
		txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
		first_txd =	(DMA_TX_DESC *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif
		
		//skb @	the	latest tx_skb
		tx_skb[nicTx->txIdx][tx_idx].skb = 0x0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->addr  = (uint32)output;
		txd->opts1 = (eor	|(((skb->len+4)	& TD_PHLEN_MASK) <<	TD_PHLEN_OFFSET));
		txd->opts1 |= FirstFrag;

		if ((skb_shinfo(skb)->nr_frags) == 0)
			first_len += 4;
		
		txd->opts2	= ((first_len &	TD_M_LEN_MASK) << TD_M_LEN_OFFSET);

		txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now
		txd->opts3 = (TD_L3CS_MASK | TD_L4CS_MASK);

		if(skb_gso_size)
			txd->opts4 |= TD_LSO_MASK;

		/* refer to RTL8198E_LSO_spec_v1_150311.doc
		   no matter direct_tx or hw_lookup,
		   need to check payload more and set txd more.
		 */
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);

		txd->opts5 =((skb_gso_size<<TD_MSS_OFFSET)| (5<<TD_IPV4_HLEN_OFFSET) |	(8));

		txd->opts1 |= (TD_BRIDGE_MASK |	TD_HWLKUP_MASK);
		txd->opts5 |= (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);

		#ifdef _LOCAL_TX_DESC
		first_txd = (DMA_TX_DESC *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
				(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}
	else
	{
		printk("\n !!! TODO: need to support this kind of skb !!!\n");
	}

	for	(cur_frag =	0; cur_frag	< skb_shinfo(skb)->nr_frags; cur_frag++)
	{
		skb_frag_t *frag = &skb_shinfo(skb)->frags[cur_frag];
		void *addr;

		addr = ((void *) page_address(frag->page.p)) + frag->page_offset;
		_dma_cache_wback((unsigned long)addr,frag->size);

		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);
		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= (DMA_TX_DESC *)&local_txd;
		memset((void *)txd, 0, sizeof(DMA_TX_DESC));
		#else
		// todo: error check, if txd = NULL....
		txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
		if (first_txd == NULL)
			first_txd =	(DMA_TX_DESC *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		tx_skb[nicTx->txIdx][tx_idx].skb = 0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->addr  = (uint32)addr;
		txd->opts1 = (eor |(((skb->len+4) &	TD_PHLEN_MASK) << TD_PHLEN_OFFSET));

		if (cur_frag == ((skb_shinfo(skb)->nr_frags) - 1))
			txd->opts2 = (((frag->size + 4) & TD_M_LEN_MASK) << TD_M_LEN_OFFSET);
		else
			txd->opts2 = ((frag->size &	TD_M_LEN_MASK) << TD_M_LEN_OFFSET);

		txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now
		txd->opts3 = (TD_L3CS_MASK	| TD_L4CS_MASK);

		if(skb_gso_size)
			txd->opts4 |= TD_LSO_MASK;

		/* refer to RTL8198E_LSO_spec_v1_150311.doc
		   no matter direct_tx or hw_lookup,
		   need to check payload more and set txd more.
		 */
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);

		txd->opts5 =((skb_gso_size<<TD_MSS_OFFSET)| (5<<TD_IPV4_HLEN_OFFSET) |	(8));
		txd->opts1 |= (TD_BRIDGE_MASK |	TD_HWLKUP_MASK);
		txd->opts5 |= (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);

		#ifdef _LOCAL_TX_DESC
		txd = (DMA_TX_DESC *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
				(uint32 *)txd, sizeof(DMA_TX_DESC) / 4);
		#else
		/* note: cache_wback of	output data	has	been done in caller	*/
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		txd->opts1 |= DESC_SWCORE_OWNED; //	set	own	bit	after all done.

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}

	#ifdef _LOCAL_TX_DESC
	if ((skb_shinfo(skb)->nr_frags) == 0)
		first_txd->opts1 |=	LastFrag;
	else
	#endif		
		txd->opts1 |= LastFrag;
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	// set own bit of first	tx_pkthdr after	all	done.
	first_txd->opts1 |=	DESC_SWCORE_OWNED;
	
	/* Set TXFD	bit	to start send */
	REG32(CPUICR) |= TXFD;

	return ret;
}
#endif

__IRAM_FWD
int32 New_swNic_send(void *skb, void * output, uint32 len,unsigned int vid, unsigned int pid)
{		
	int	ret;
	unsigned long flags	= 0;

	SMP_LOCK_ETH_XMIT(flags);
	ret	= _New_swNic_send(skb, output, len, vid, pid);
	SMP_UNLOCK_ETH_XMIT(flags);
	return ret;
}

#ifdef CONFIG_RTL_TSO
__IRAM_FWD
int32 New_swNic_send_tso_sg(void *skb, void	* output, uint32 len, rtl_nicTx_info *nicTx)
{
	int	ret;
	unsigned long flags	= 0;

	SMP_LOCK_ETH_XMIT(flags);
	ret	= _New_swNic_send_tso_sg(skb, output, len, nicTx);
	SMP_UNLOCK_ETH_XMIT(flags);
	return ret;
}
#endif

/*
  Why to use HW	CDP	instead	of own bit (i.e. use USE_SWITCH_TX_CDP flag)?
  Reason:
	the	cache line of 8197F	is 32-byte.
	the	descriptor size	is 24-byte and can not pad to 32-byte because
		HW CDP will	move to	next 24-byte after it Rx/Tx	a pkt.
	when _DESC_CACHE_ACCESS_TX is defined, for example:
		current	Tx index is	5,
		previous Tx	index is 4,	in cacheable address the own bit is	1(SWCORE ownd),
			IN CASE	HW is TX done this pkt,	it will	set	own	bit	to 0 (CPU ownd)	in physical	memory,
		for	current	Tx pkt (idx:5),	it will	do cache writeback the desc	(24-byte) to physical memory,
			it will	ALSO writeback previous	desc's own bit from	cacheable address (own bit=1) to
			physical memory	(own bit=0).
		when New_swNic_txDone()	run	to the "checking of	idx	4",	the	own	bit	will NEVER equal to	"CPU ownd".
			so the Tx path is failed.
  i.e. MUST	define USE_SWITCH_TX_CDP when use _DESC_CACHE_ACCESS_TX
 */
__IRAM_FWD
int32 New_swNic_txDone(void)
{
	int idx = 0;
	unsigned long flags=0;
	struct sk_buff *skb;
	#ifdef USE_SWITCH_TX_CDP
	uint32 hw_cdp_data;
	int	hw_cdp_idx;
	#else
	DMA_TX_DESC	*tx_desc;
	#endif

	SMP_LOCK_ETH_XMIT(flags);

	#ifdef USE_SWITCH_TX_CDP
	hw_cdp_data	= REG32(TxCDP_reg[idx]);
	hw_cdp_idx = (hw_cdp_data -	New_txDescRing_base[idx]) /	sizeof(struct dma_tx_desc);
	
	while (New_txPktDoneDescIndex[idx] != hw_cdp_idx) {

		skb	= (struct sk_buff *)tx_skb[idx][New_txPktDoneDescIndex[idx]].skb;

		if (skb)
		{
			#ifdef __ECOS
			rtl_dev_kfree_skb_any(skb);
			#else
			dev_kfree_skb_any(skb);
			#endif
			tx_skb[idx][New_txPktDoneDescIndex[idx]].skb = 0;
		}

		if (++New_txPktDoneDescIndex[idx] == New_txDescRingCnt[idx])
			New_txPktDoneDescIndex[idx]	= 0;
	}

	#else
	while (New_txPktDoneDescIndex[idx] != New_currTxPkthdrDescIndex[idx]) {

		tx_desc	= &New_txDescRing[idx][New_txPktDoneDescIndex[idx]];
		// or, use _dma_cache_inv((unsigned	long)tx_desc,(unsigned long)sizeof(struct dma_tx_desc));
		tx_desc	= (DMA_TX_DESC *)(((uint32)tx_desc)	| UNCACHE_MASK);

		if ((tx_desc->opts1	& DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			skb	= (struct sk_buff *)tx_skb[idx][New_txPktDoneDescIndex[idx]].skb;

			if (skb)
			{
				#ifdef __ECOS
				rtl_dev_kfree_skb_any(skb);
				#else
				dev_kfree_skb_any(skb);
				#endif

				tx_skb[idx][New_txPktDoneDescIndex[idx]].skb = 0;
			}

			if (++New_txPktDoneDescIndex[idx] == New_txDescRingCnt[idx])
				New_txPktDoneDescIndex[idx]	= 0;
		}
		else
			break;
	}
	#endif

	SMP_UNLOCK_ETH_XMIT(flags);
	return 0;
}

void New_swNic_freeRxBuf(void)
{
	int	i,j;

	/* Initialize index	of Tx pkthdr descriptor	*/
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
		New_currTxPkthdrDescIndex[i] = 0;
		New_txPktDoneDescIndex[i]=0;
	}

	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		if (rx_skb[i]){
			for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
			{
				void *skb;

				skb	= (void	*)rx_skb[i][j].skb;
				if (skb)
				{
					free_rx_buf(skb);
					rx_skb[i][j].skb = 0;
				}
			}
		}
		/* Initialize index	of current Rx pkthdr descriptor	*/
		New_currRxPkthdrDescIndex[i] = 0;
	}
}

/* ===========================================================================
 *
 *			Backup code
 *
 * ===========================================================================
 */
// previous version, not easy to read
// use this version of New_swNic_receive(), and enable _DESC_CACHE_ACCESS_TX and USE_SWITCH_TX_CDP,
//		it seems okay, still do not know why.
#if 0
#define	SET_OWN_BIT(desc) \
	desc->opts1	|= DESC_SWCORE_OWNED;

#define	INC_IDX(ring, idx) \
	idx	= NEXT_IDX(idx,New_rxDescRingCnt[ring]);	\
	New_currRxPkthdrDescIndex[ring]	= idx;

#define	REUSE_DESC() \
	SET_OWN_BIT(desc); \
	INC_IDX(ring_idx, rx_idx); 

int32 New_swNic_receive(rtl_nicRx_info *info, int retryCount)
{
	uint32 rx_idx, ring_idx;
	struct sk_buff *r_skb;
	DMA_RX_DESC	*desc;
	unsigned char *buf;
	void *skb;

	#ifdef USE_SWITCH_RX_CDP
	uint32 cdp_value;
	int	hw_idx;
	#endif

	#if	!defined(CONFIG_SMP)
	unsigned long flags	= 0;
	SMP_LOCK_ETH_RECV(flags);
	#endif

	#if	defined(RTL_MULTIPLE_RX_TX_RING)
	for(ring_idx = (NEW_NIC_MAX_RX_DESC_RING-1);ring_idx >=	(info->priority);ring_idx--)
	#else
	for(ring_idx=0;ring_idx<1;ring_idx++)
	#endif
	{
get_next:
		rx_idx = New_currRxPkthdrDescIndex[ring_idx];

		#ifdef USE_SWITCH_RX_CDP
		cdp_value =	REG32(CPURPDCR(ring_idx));
		hw_idx = (cdp_value	- New_rxDescRing_base[ring_idx]) / sizeof(struct dma_rx_desc);
		if (rx_idx == hw_idx) {
			continue; // go	for	next rx	ring
		}
		#endif

		desc = &New_rxDescRing[ring_idx][rx_idx];

		// NEEDED, do not removed. when	previous interrupt_dsr_rx()	is finished,
		//		the	New_rxDescRing[ring_idx][rx_idx] has been read,	and	found "no more packet",
		//		so its address is cached.
		//		if _dma_cache_inv()	removed, and one packet	come in, read the desc and it own bit
		//		is still SWCORE	owned.
		// TODO: need to test, remove this one,	and	add	_dma_cache_inv to the next IF (own bit is SWCORE owned)

		// todo: in	FPGA, when rx_desc use cacheable and do	_dma_cache_inv,	the	rx ring	will abnormal
		#ifdef _DESC_CACHE_ACCESS_RX
		_dma_cache_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
		#else
		desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
		#endif

		#ifndef	USE_SWITCH_RX_CDP
		//1.check desc's own bit
		if (desc->opts1	& DESC_OWNED_BIT)//1: SWCORE owned
		{
			continue; // go	for	next rx	ring
		}
		#endif

		// if checksum failed, reuse this desc, except the ipv6 ready logo case.
		if ((desc->opts5 & (RD_L3CSOK_MASK | RD_L4CSOK_MASK)) != (RD_L3CSOK_MASK | RD_L4CSOK_MASK))
		{
			#ifdef CONFIG_RTL_IPV6READYLOGO
			if (!(((desc->opts5&(RD_L4CSOK_MASK))!=RD_L4CSOK_MASK) && 
				((desc->opts4&RD_IPV6_MASK)==RD_IPV6_MASK) && ((desc->opts3&RD_REASON_MASK)==0x60)))
			#endif	
			{
				#ifdef _DESC_CACHE_ACCESS_RX
				desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
				#endif
				REUSE_DESC();
				goto get_next;
			}
		}

		// porting from swNic_receive(), need or not ??
		#if 0 //defined(CONFIG_RTL_HARDWARE_NAT)
		if (((desc->opts4 & RD_SPA_MASK) >> RD_SPA_OFFSET) >= RTL8651_CPU_PORT) {
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}
		#endif
		
		//2.rx skb
		r_skb =	(struct	sk_buff	*)(rx_skb[ring_idx][rx_idx].skb);
		if (r_skb == NULL) {	// should not happen
			// todo: may call alloc_rx_buf() to	get	a new skb,....
			printk("ETH_ERR: skb_array[%d][%d] is NULL\n",ring_idx,rx_idx);
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}

		//3.alloc new rx skb
		buf	= alloc_rx_buf(&skb, size_of_cluster);
		if (buf) {
			//update to	info struct
			//info->priority = ring_idx; //	todo
			info->vid =	(desc->opts4 & RD_DVLANID_MASK)	>>	RD_DVLANID_OFFSET;
			info->pid =	(desc->opts4 & RD_SPA_MASK)	>> RD_SPA_OFFSET;
			info->rxPri	= (desc->opts5 & RD_DPRI_MASK) >> RD_DPRI_OFFSET;
			r_skb->len = (desc->opts2 &	RD_LEN_MASK) - 4;
			info->input	= r_skb;
			info->len =	r_skb->len;
			// todo, update	the	other field: priority, ...

			/* after enable	Rx scatter gather feature (0xb80100a4 bit5 cf_rx_gather	= 1),
			 * the mdata of	second/third/... pkthdr	will be	updated	to 4-byte alignment	by hardware.
			 */
			r_skb->data	= (unsigned	char *)(desc->addr);

			_dma_cache_inv((unsigned long)r_skb->data, r_skb->len);

			//4.hook
			rx_skb[ring_idx][rx_idx].skb = (uint32)skb;

			#ifdef _DESC_CACHE_ACCESS_RX
			//desc->addr = (uint32)buf;
			//_dma_cache_wback_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			desc->addr = (uint32)buf;
			#else
			desc->addr = (uint32)buf;
			#endif

			//5.set	own	bit, move down
			SET_OWN_BIT(desc);

			//6. increase rx index
			INC_IDX(ring_idx, rx_idx);

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_OK;
		}
		else {
			// just	leave this packet in rx	desc
			// do not change own_bit and rx_index
			#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
			rx_noBuffer_cnt++;
			#endif

			#if	defined(CONFIG_RTL_ETH_PRIV_SKB)
			printk("ETH_ERR: no	private	skb	buffer\n");
			#else
			printk("ETH_ERR: kernel	allocate skb failed\n");
			#endif

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_NULL;
		}
	}

	#if	!defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
	#endif
	return (RTL_NICRX_NULL);
}

static inline void fill_txd_misc(DMA_TX_DESC *txd, rtl_nicTx_info *nicTx, struct sk_buff *skb)
{
	// extract from	_swNic_send, purpose: keep a function shortly and easy to read
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	txd->opts3 = (txd->opts3 & ~TD_DPRI_MASK) |	(nicTx->priority<<TD_DPRI_OFFSET);
	#endif

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| nicTx->tagport;
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);
	#endif

	#ifdef CONFIG_RTL_VLAN_8021Q
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| (0x3f	& rtk_get_vlan_tagmask(nicTx->vid));
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);

	#elif defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#ifdef	CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
			#ifdef CONFIG_RTL_8367R_SUPPORT
			rtk_get_real_nicTxVid(skb, &nicTx->vid);
			txd->opts3 = (txd->opts3 & ~TD_DVLANID_MASK) | nicTx->vid;
			#endif
			txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| (0x3f	& rtk_getTagPortmaskByVid(nicTx->vid,(void *)skb->data));
		#else
			txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| auto_set_tag_portmask;
		#endif
	}
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);
	#endif
}

static inline void fill_txd_tso(DMA_TX_DESC	*txd, char *data)
{
	/* TODO:
		check dev->features's NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT
		bits(set in	re865x_probe), if true then	set	these 2	bits of	the	Tx pktHdr,
		NOT	always set.

		due	to we enable NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT bits
		of dev->features in	re865x_probe(),	so we must enable hardware L3/L4 checksum offload feature.
	 */
	uint16 type	= htons( *(uint16 *)(data+12) );

	txd->opts3 |= (TD_L3CS_MASK	| TD_L4CS_MASK);

	// TODO: the type is listed	?? vi/li/pi/...	need to	be checked
	if (type ==	ETH_P_IPV6)	{
		txd->opts1 |= (PKTHDR_IPV6 << TD_TYPE_OFFSET);
		txd->opts3 |= TD_IPV6_MASK;
	}
	else if	(type == ETH_P_IP) {
		struct iphdr *iph;

		iph	= (struct iphdr	*)(data+14);

		if (iph->protocol == IPPROTO_TCP)
			txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_UDP)
			txd->opts1 |= (PKTHDR_UDP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_ICMP)
			txd->opts1 |= (PKTHDR_ICMP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_IGMP)
			txd->opts1 |= (PKTHDR_IGMP << TD_TYPE_OFFSET);
		else
			txd->opts1 |= (PKTHDR_IP <<	TD_TYPE_OFFSET);
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
	}
	else {
		txd->opts1 |= (PKTHDR_ETHERNET << TD_TYPE_OFFSET);
	}
}

/*************************************************************************
*	FUNCTION
*		_New_swNic_send
*
*	DESCRIPTION
*		This function writes one packet	to tx descriptors, and waits until
*		the	packet is successfully sent.
*
*	INPUTS
*		None
*
*	OUTPUTS
*		None
*************************************************************************/

int32 _New_swNic_send(void *skb, void *output, uint32 len, rtl_nicTx_info *nicTx)
{
	uint32 tx_idx, eor,	ret	= SUCCESS;
	DMA_TX_DESC	*txd;
	uint32 next_index;

	tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];

	next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

	if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
		/* TX ring full	*/
		return(FAILED);
	}

	txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
	// txd should NOT be NULL, otherwise ....

	#ifndef	_DESC_CACHE_ACCESS_TX
	txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
	#endif

	// store the skb pointer for New_swNic_txDone later
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;
	eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;

	/* Pad small packets and hardware will add CRC */
	if ( len < 60 )
		len	= 64;
	else
		len	+= 4;

	txd->addr  = ((uint32)output);
	txd->opts1 = (eor  |FirstFrag |LastFrag	|((len & TD_PHLEN_MASK)	<< TD_PHLEN_OFFSET));
	txd->opts2 = ((len & TD_M_LEN_MASK)	<< TD_M_LEN_OFFSET);
	txd->opts3 = nicTx->vid;
	txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);

	if ((nicTx->flags &	PKTHDR_HWLOOKUP) !=	0) {
		txd->opts1 |= ((1<<TD_BRIDGE_OFFSET) | (1<<TD_HWLKUP_OFFSET));
		txd->opts5 = (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);
	}

	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	fill_txd_misc(txd, nicTx, (struct sk_buff *)skb);
	#endif

	#ifdef CONFIG_RTL_TSO
	fill_txd_tso(txd, (char	*)output);
	#endif

	/* note: cache_wback of	output data	has	been done in caller	*/
	#ifdef _DESC_CACHE_ACCESS_TX
	_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
	txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
	#endif

	txd->opts1 |= DESC_SWCORE_OWNED; //	set	own	bit	after all done.

	New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;

	/* Set TXFD	bit	to start transmission */
	REG32(CPUICR) |= TXFD;

	return ret;
}

int32 _New_swNic_send_tso_sg(struct	sk_buff	*skb, void * output, uint32	len,rtl_nicTx_info *nicTx)
{
	uint32 tx_idx =	0, ret = SUCCESS; //, eor =	0;
	struct tx_desc	*txd=NULL, *first_txd=NULL;
	uint32 next_index;
	uint32 cur_frag;
	uint32 first_len;
	uint32 skb_gso_size	= skb_shinfo(skb)->gso_size;

	first_len =	skb->len - skb->data_len;

	if(first_len)
	{
		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= &local_txd;
		#else
		txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];

		first_txd =	(struct tx_desc *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		memset((void *)txd, 0, sizeof(struct tx_desc));

		//skb @	the	latest tx_skb
		tx_skb[nicTx->txIdx][tx_idx].skb = 0x0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->mdata  = (uint32)output;
		txd->tx_ph_len = skb->len+4;
		txd->tx_fs = 1;
		if ((skb_shinfo(skb)->nr_frags) == 0)
			first_len += 4;
		txd->tx_mlen = first_len;

		// due to enable HW Tx CSUM, always use hw lookup, no need to assign tx_dp 
		//txd->tx_dp = nicTx->portlist;	// direct tx now
		txd->tx_l3cs = 1;
		txd->tx_l4cs = 1;

		if(skb_gso_size)
			txd->tx_lso = 1;

		// use hw lookup, no need to assign tx_dp 
		/***
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
		***/
		txd->tx_type = PKTHDR_TCP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;		
		txd->tx_mss = skb_gso_size;

		/* TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
		   		 how can it work?
		 */
		txd->tx_hwlkup = 1;
		txd->tx_bridge = 1;
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;

		#ifdef _LOCAL_TX_DESC
		// we will set own bit in first_txd later
		first_txd = (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
					(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else

		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}
	else
	{
		printk("\n !!! TODO: need to support this kind of skb !!!\n");
	}

	for	(cur_frag =	0; cur_frag	< skb_shinfo(skb)->nr_frags; cur_frag++)
	{
		skb_frag_t *frag = &skb_shinfo(skb)->frags[cur_frag];
		void *addr;

		addr = ((void *) page_address(frag->page.p)) + frag->page_offset;
		_dma_cache_wback((unsigned long)addr,frag->size);

		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);
		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= &local_txd;
		#else
		txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];
		//if (first_txd == NULL)
		//	first_txd =	(struct tx_desc *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		memset((void *)txd, 0, sizeof(struct tx_desc));

		tx_skb[nicTx->txIdx][tx_idx].skb = 0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;

		txd->mdata  = (uint32)addr;
		txd->tx_ph_len = skb->len+4;
		if (cur_frag == ((skb_shinfo(skb)->nr_frags) - 1))
			txd->tx_mlen = frag->size + 4;
		else
			txd->tx_mlen = frag->size;

		//txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now
		txd->tx_l3cs = 1;
		txd->tx_l4cs = 1;

		if(skb_gso_size)
			txd->tx_lso = 1;

		/***
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
		***/
		txd->tx_type = PKTHDR_TCP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;
		txd->tx_mss = skb_gso_size;

		/* TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
		   		 how can it work?
		 */
		txd->tx_hwlkup = 1;
		txd->tx_bridge = 1;
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;

		#ifdef _LOCAL_TX_DESC
		txd = (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
					(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		txd->tx_own = 1; //	set	own	bit	after all done.

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}

	//if (first_txd == NULL) // should not happen
	//	return(FAILED);

	#ifdef _LOCAL_TX_DESC
	if ((skb_shinfo(skb)->nr_frags) == 0)
		first_txd->tx_ls = 1;
	else
	#endif		
		txd->tx_ls = 1;
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	// set own bit of first	tx_pkthdr after	all	done.
	first_txd->tx_own = 1;
	
	/* Set TXFD	bit	to start send */
	REG32(CPUICR) |= TXFD;

	return ret;
}
#endif

