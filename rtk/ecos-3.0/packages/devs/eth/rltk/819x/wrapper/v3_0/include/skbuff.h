 /******************************************************************************
  *
  * Name: skbuff.h - header file Linux skb wrapper
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#ifndef __SKBUFF_H__
#define __SKBUFF_H__

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <string.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>

#if defined(CONFIG_RTL_VLAN_SUPPORT)
#include <netinet/rtl_vlan.h>
#endif

#if 0 /*HF. keep skb num >= cluster num*/
#if defined(RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0) && defined(RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN1)
#define MAX_LOCAL_SKB_NUM		2048
#else
//#define MAX_LOCAL_SKB_NUM		640
#define MAX_LOCAL_SKB_NUM		1024
//#define MAX_LOCAL_SKB_NUM		1536
#endif
#else
#define MAX_LOCAL_SKB_NUM		((CYGPKG_NET_CLUSTERS_SIZE/MCLBYTES)+50)
#endif

#ifdef TX_SCATTER		
//#define MAX_LIST_NUM			3
#define MAX_LIST_NUM			1
#endif

#define __BIG_ENDIAN_BITFIELD

//#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
#if defined(CONFIG_RTL_819X)
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>

struct pppoe_tag {
      __u16 tag_type;
      __u16 tag_len;
      char tag_data[0];
 } __attribute ((packed));
 
struct pppoe_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8 ver : 4;
        __u8 type : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
        __u8 type : 4;
        __u8 ver : 4;
#endif
       __u8 code;
        __u16 sid;
        __u16 length;
        struct pppoe_tag tag[0];
} __attribute ((packed));


struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8    ihl:4,
                version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
        __u8    version:4,
               ihl:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
         __u8    tos;
         __u16   tot_len;
         __u16   id;
         __u16   frag_off;
         __u8    ttl;
         __u8    protocol;
         __u16   check;
         __u32   saddr;
         __u32   daddr;
        /*The options start here. */
};

#define IPX_NODE_LEN    6

typedef struct
{
        __u32   net;
        __u8    node[IPX_NODE_LEN]; 
        __u16   sock;
} ipx_address;

struct ipxhdr
{
        __u16           ipx_checksum __attribute__ ((packed));
#define IPX_NO_CHECKSUM 0xFFFF
        __u16           ipx_pktsize __attribute__ ((packed));
        __u8            ipx_tctrl;
        __u8            ipx_type;
#define IPX_TYPE_UNKNOWN        0x00
#define IPX_TYPE_RIP            0x01    /* may also be 0 */
#define IPX_TYPE_SAP            0x04    /* may also be 0 */
#define IPX_TYPE_SPX            0x05    /* SPX protocol */
#define IPX_TYPE_NCP            0x11    /* $lots for docs on this (SPIT) */
#define IPX_TYPE_PPROP          0x14    /* complicated flood fill brdcast */
        ipx_address     ipx_dest __attribute__ ((packed));
        ipx_address     ipx_source __attribute__ ((packed));
};


#if defined(__ECOS)
#pragma pack(1)
#endif

/* AppleTalk AARP headers */

struct elapaarp
{
        __u16   hw_type;
#define AARP_HW_TYPE_ETHERNET           1
#define AARP_HW_TYPE_TOKENRING          2
        __u16   pa_type;
        __u8    hw_len;
        __u8    pa_len;
#define AARP_PA_ALEN                    4
        __u16   function;
#define AARP_REQUEST                    1
#define AARP_REPLY                      2
#define AARP_PROBE                      3
        __u8    hw_src[ETH_ALEN];
        __u8    pa_src_zero;
        __u16   pa_src_net;
        __u8    pa_src_node;
        __u8    hw_dst[ETH_ALEN];
        __u8    pa_dst_zero;
        __u16   pa_dst_net;
        __u8    pa_dst_node;       
};

#if defined(__ECOS)
#pragma pack()
#endif


 struct ddpehdr
 {
 #ifdef __LITTLE_ENDIAN_BITFIELD
         __u16   deh_len:10, deh_hops:4, deh_pad:2;
 #else
         __u16   deh_pad:2, deh_hops:4, deh_len:10;
 #endif
         __u16   deh_sum;
         __u16   deh_dnet;
         __u16   deh_snet;
         __u8    deh_dnode;
         __u8    deh_snode;
         __u8    deh_dport;
         __u8    deh_sport;
         /* And netatalk apps expect to stick the type in themselves */
 };
 
struct udp_hdr {
        __u16   source;
        __u16   dest;
        __u16   len;
        __u16   check;
};
#endif


struct icmp6hdr {
	   u8 		   icmp6_type;
	   u8 		   icmp6_code;
	   u16		   icmp6_cksum;
	   union {
			   u32				   un_data32[1];
			   u16				   un_data16[2];
			   u8 				   un_data8[4];
			   struct icmpv6_echo {
					   u16		   identifier;
					   u16		   sequence;
			   } u_echo;
			   
			   struct icmpv6_nd_advt {
#if defined(__LITTLE_ENDIAN_BITFIELD)
					   u32		   reserved:5,
									   override:1,
									   solicited:1,
									   router:1,
									   reserved2:24;
#elif defined(__BIG_ENDIAN_BITFIELD)
					   u32		   router:1,
									   solicited:1,
									   override:1,
									   reserved:29;
#else
#error  "Please fix <asm/byteorder.h>"
#endif										   
			   } u_nd_advt;

			   struct icmpv6_nd_ra {
					   u8 		   hop_limit;
#if defined(__LITTLE_ENDIAN_BITFIELD)
					   u8 		   reserved:6,
									   other:1,
									   managed:1;

#elif defined(__BIG_ENDIAN_BITFIELD)
					   u8 		   managed:1,
									   other:1,
									   reserved:6;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
					   u16		   rt_lifetime;
			   } u_nd_ra;

		} icmp6_dataun;
};	   


struct net_device_stats {
	unsigned long   rx_packets;             /* total packets received       */
	unsigned long   tx_packets;             /* total packets transmitted    */
	unsigned long   rx_bytes;               /* total bytes received         */
	unsigned long   tx_bytes;               /* total bytes transmitted      */
	unsigned long   rx_errors;              /* bad packets received         */
	unsigned long   tx_errors;              /* packet transmit problems     */
	unsigned long   rx_dropped;             /* no space in linux buffers    */
	unsigned long   tx_dropped;             /* no space available in linux  */
	unsigned long   multicast;              /* multicast packets received   */
	unsigned long   collisions;

	/* detailed rx_errors: */
	unsigned long   rx_length_errors;
	unsigned long   rx_over_errors;         /* receiver ring buff overflow  */
	unsigned long   rx_crc_errors;          /* recved pkt with crc error    */
	unsigned long   rx_frame_errors;        /* recv'd frame alignment error */
	unsigned long   rx_fifo_errors;         /* recv'r fifo overrun          */
	unsigned long   rx_missed_errors;       /* receiver missed packet       */

	/* detailed tx_errors */
	unsigned long   tx_aborted_errors;
	unsigned long   tx_carrier_errors;
	unsigned long   tx_fifo_errors;
	unsigned long   tx_heartbeat_errors;
	unsigned long   tx_window_errors;

	/* for cslip etc */
	unsigned long   rx_compressed;
	unsigned long   tx_compressed;
};

#ifdef TX_SCATTER
struct buf_list {
 	unsigned int len;
	unsigned char *buf;
};
#endif

struct skb_shared_info {
	atomic_t        dataref;
};

struct  sk_buff_head {
	struct list_head *next, *prev;
	u32           qlen;
};

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
#define skb_shinfo(SKB)	((struct skb_shared_info *)((SKB)->ref))
#endif

struct sk_buff {
	/* These two members must be first. */
	struct sk_buff  * next;		/* Next buffer in list                          */
	struct sk_buff  * prev;		/* Previous buffer in list                      */
	struct sk_buff_head *list;	/* List we are on */	
	unsigned char	*head;		/* Head of buffer */
	unsigned char	*data;		/* Data head pointer */
	unsigned char	*tail;		/* Tail pointer	*/
	unsigned char  *end;		/* End pointer */ 
	struct net_device *dev;	/* Device we arrived on/are leaving by */	
	unsigned long key;		/* Tx Key id of ecos driver */	
	unsigned int 	len;			/* Length of actual data */	
	/*must aligned on 4 bytes to reduce unaligned load/store. just place after int type -- * HF 20130201
    if modified the size of cb,remember to sync the size to pkthdr(Mpbuf.h) --jerry_ge 20131231
	  */  
	char		    cb[24];
#if defined(CONFIG_RTL_VLAN_SUPPORT)
		struct vlan_tag tag;
#if defined(CONFIG_RTL_BRIDGE_VLAN_SUPPORT)
		unsigned char forward_rule:2,/* lan <-> wan forward rule:1 napt, 2 bridge */
							index:2,/* indicate skb is cloned or not in rx vlan process  */
							flag_src:2,/* indicate store lan vlan info used for wan */
							taged:2;/* indicate tag or not */
	
#endif
#endif	
	unsigned char cloned;	       /* head may be cloned (check refcnt to be sure). */
#if 1
	unsigned char pppoe_flag;
	unsigned char l2tp_flag;
	unsigned char l2tp_offset;
	unsigned char pptp_flag;
	unsigned char pptp_offset;
#endif

#if defined(CONFIG_RTL_819X)
	unsigned char	__unused;
#endif
#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
	unsigned char wanorlan; /* wan:1 lan:2 default:0 */
#endif

#ifdef TX_SCATTER
	unsigned int 	total_len;	/* Total length */	        
	int		list_idx;		/* current used buf index */
	int		list_num;	/* total list buf num */
	struct buf_list	 list_buf[MAX_LIST_NUM];
#endif
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	atomic_t	data_ref;
	unsigned char	*ref;
#endif
#if defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
	__u16			srcPort;
	__u16			srcVlanId:12;
#endif
#ifdef CONFIG_RTL_DELAY_REFILL
	void *priv; 	/*NULL means from NIC, not NULL means wifi root priv*/
#endif
};

struct net_device {
	char	name[16];
	void	*priv;	     /* pointer to private data	*/
	void *info;		/* pointer to ecos driver private */
	unsigned char	dev_addr[6]; /* set during bootup */
	void    *pci_dev;
	int (*init)(void);
	int (*open)(struct net_device *dev);
	int (*stop)(struct net_device *dev);
	int (*hard_start_xmit)(struct sk_buff *skb, struct net_device *dev);
	int (*can_xmit)(struct net_device *dev);
	int (*do_ioctl)(struct net_device *dev, struct ifreq *ifr, int cmd);
	int (*isr)(struct net_device *dev);
	void (*dsr)(struct net_device *dev);
	struct net_device_stats* (*get_stats)(struct net_device *dev);
	unsigned short flags;      /* flag used by device	*/
	unsigned long	base_addr;	/* device I/O address	*/
	unsigned char	irq; 		/* device IRQ number    */
  	unsigned long	last_rx;	/* Time of last Rx	*/
};

static inline int netif_running(struct net_device *dev)
{
	if (dev->flags)
		return 1;
	else
		return 0;
}

/* Copy sk_buff struct */
static inline void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	memcpy(new->cb, old->cb, sizeof(old->cb));
}

static inline unsigned char *__skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp=skb->tail;
	skb->tail+=len;
	skb->len+=len;

#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len += len;
#endif
	
	return tmp;
}

/**
 *	skb_cloned - is the buffer a clone
 *	@skb: buffer to check
 *
 *	Returns true if the buffer was generated with skb_clone() and is
 *	one of multiple shared copies of the buffer. Cloned buffers are
 *	shared data so must not be written to under normal circumstances.
 */
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
int skb_cloned(struct sk_buff *skb);
#else
static inline int skb_cloned(struct sk_buff *skb)
{
	return skb->cloned && atomic_read(&skb_shinfo(skb)->dataref) != 1;
}
#endif

/**
 *	skb_put - add data to a buffer
 *	@skb: buffer to use 
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned.
 */
 
static inline unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp=skb->tail;
	skb->tail+=len;
	skb->len+=len;
	if(skb->tail>skb->end) {
		ASSERT(0);		
	}

#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len += len;
#endif
	
	return tmp;
}

static inline unsigned char *__skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data-=len;
	skb->len+=len;

#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len += len;
#endif
	
	return skb->data;
}

/**
 *	skb_push - add data to the start of a buffer
 *	@skb: buffer to use 
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned.
 */

static inline unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data-=len;
	skb->len+=len;
	if ((long)skb->data< (long)skb->head) 		
		ASSERT(0);
	
#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len += len;
#endif
	
	return skb->data;
}

static inline unsigned char *__skb_pull(struct sk_buff *skb, unsigned int len)
{
	skb->len-=len;
	skb->data = (unsigned char *)(((unsigned int)skb->data) + len);

#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len -= len;
#endif
	
	return skb->data;
}

/**
 *	skb_pull - remove data from the start of a buffer
 *	@skb: buffer to use 
 *	@len: amount of data to remove
 *
 *	This function removes data from the start of a buffer, returning
 *	the memory to the headroom. A pointer to the next data in the buffer
 *	is returned. Once the data has been pulled future pushes will overwrite
 *	the old data.
 */
/*
static inline unsigned char * skb_pull(struct sk_buff *skb, unsigned int len)
{	
	if (len > skb->len)
		return NULL;
	return __skb_pull(skb,len);
}
*/
unsigned char * skb_pull(struct sk_buff *skb, unsigned int len);

/**
 *	skb_reserve - adjust headroom
 *	@skb: buffer to alter
 *	@len: bytes to move
 *
 *	Increase the headroom of an empty &sk_buff by reducing the tail
 *	room. This is only allowed for an empty buffer.
 */

static inline void skb_reserve(struct sk_buff *skb, unsigned int len)
{
	skb->data+=len;
	skb->tail+=len;

#ifdef TX_SCATTER
	if (skb->list_num && skb->total_len)
		skb->total_len += len;
#endif	
}


/**
 *      __skb_dequeue - remove from the head of the queue
 *      @list: list to dequeue from
 *
 *      Remove the head of the list. This function does not take any locks
 *      so must be used with appropriate locks held only. The head item is
 *      returned or %NULL if the list is empty.
 */

static inline struct sk_buff *__skb_dequeue(struct sk_buff_head *list)
{
        struct sk_buff *next, *prev, *result;

        prev = (struct sk_buff *) list;
        next = prev->next;
        result = NULL;
        if (next != prev) {
                result = next;
                next = next->next;
                list->qlen--;
                next->prev = prev;
                prev->next = next;
                result->next = NULL;
                result->prev = NULL;
                result->list = NULL;
        }
        return result;
}

static inline struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	u32 x;

	save_and_cli(x);
	skb = __skb_dequeue(list);
	restore_flags(x);
	
	return skb;
}


/**             
 *      skb_trim - remove end from a buffer
 *      @skb: buffer to alter
 *      @len: new length
 *              
 *      Cut the length of a buffer down by removing data from the tail. If
 *      the buffer is already under the length specified it is not modified.
 */             
                
static inline void skb_trim(struct sk_buff *skb, unsigned int len)
{               
	skb->len = len;
	skb->tail = skb->data + len;

}     

/**
 *      skb_queue_len   - get queue length
 *      @list_: list to measure
 *
 *      Return the length of an &sk_buff queue. 
 */
 
static inline u32 skb_queue_len(struct sk_buff_head *list_)
{
        return(list_->qlen);
}

static inline void skb_queue_head_init(struct sk_buff_head *list)
{
        list->prev = (struct list_head *)list;
        list->next = (struct list_head *)list;
        list->qlen = 0;
}

/**
 *      skb_headroom - bytes at buffer head
 *      @skb: buffer to check
 *
 *      Return the number of bytes of free space at the head of an &sk_buff.
 */
 
static inline int skb_headroom(const struct sk_buff *skb)
{
        return skb->data-skb->head;
}


/*
 *      Insert an sk_buff at the start of a list.
 *
 *      The "__skb_xxxx()" functions are the non-atomic ones that
 *      can only be called with interrupts disabled.
 */

/**
 *      __skb_queue_head - queue a buffer at the list head
 *      @list: list to use
 *      @newsk: buffer to queue
 *
 *      Queue a buffer at the start of a list. This function takes no locks
 *      and you must therefore hold required locks before calling it.
 *
 *      A buffer cannot be placed on two lists at the same time.
 */     
 
static inline void __skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
        struct sk_buff *prev, *next;

        newsk->list = list;
        list->qlen++;
        prev = (struct sk_buff *)list;
        next = prev->next;
        newsk->next = next;
        newsk->prev = prev;
        next->prev = newsk;
        prev->next = newsk;
}

/**
 *      skb_queue_head - queue a buffer at the list head
 *      @list: list to use
 *      @newsk: buffer to queue
 *
 *      Queue a buffer at the start of the list. This function takes the
 *      list lock and can be used safely with other locking &sk_buff functions
 *      safely.
 *
 *      A buffer cannot be placed on two lists at the same time.
 */     

static inline void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
        unsigned long flags;

        save_and_cli(flags);
        __skb_queue_head(list, newsk);
        restore_flags(flags);
}


/**
 *      __skb_queue_tail - queue a buffer at the list tail
 *      @list: list to use
 *      @newsk: buffer to queue
 *
 *      Queue a buffer at the end of a list. This function takes no locks
 *      and you must therefore hold required locks before calling it.
 *
 *      A buffer cannot be placed on two lists at the same time.
 */     
 

static inline void __skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
        struct sk_buff *prev, *next;

        newsk->list = list;
        list->qlen++;
        next = (struct sk_buff *)list;
        prev = next->prev;
        newsk->next = next;
        newsk->prev = prev;
        next->prev = newsk;
        prev->next = newsk;
}

/**
 *      skb_queue_tail - queue a buffer at the list tail
 *      @list: list to use
 *      @newsk: buffer to queue
 *
 *      Queue a buffer at the tail of the list. This function takes the
 *      list lock and can be used safely with other locking &sk_buff functions
 *      safely.
 *
 *      A buffer cannot be placed on two lists at the same time.
 */     

static inline void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
        unsigned long flags;

        save_and_cli(flags);
        __skb_queue_tail(list, newsk);
        restore_flags(flags);
}

static inline void skb_assign_buf(struct sk_buff *skb, unsigned char *buf, unsigned int len)
{	
	skb->head = buf;
	skb->data = buf;
	skb->tail = buf;
	skb->end = buf + len;
}

static inline int skb_tailroom(const struct sk_buff *skb)
{
         return skb->end-skb->tail;
}
#define skb_queue_walk(queue, skb) \
                for (skb = (queue)->next;                       \
                     (skb != (struct sk_buff *)(queue));        \
                     skb=skb->next)

static inline bool skb_queue_is_last(const struct sk_buff_head *list,
                                     const struct sk_buff *skb)
{
        return (skb->next == (struct sk_buff *) list);
}

/*
 *	External functions
 */
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
extern void kfree_skb(struct sk_buff *skb);
extern void kfree_skb_chk_key(struct sk_buff *skb, struct net_device *root_dev);
extern void wlan_dev_kfree_skb_any(struct sk_buff *skb);
#ifdef ISR_DIRECT
extern void netif_rx(struct sk_buff *skb, struct net_device *root_dev);
#else
extern void netif_rx(struct sk_buff *skb);
#endif
extern void init_skb_pool(void);
extern void init_skb_data_pool(void);
#ifdef TX_SCATTER
struct sk_buff *alloc_skb_buf(unsigned char *buf, int len);
#endif
extern struct sk_buff *alloc_skb(int size);
extern struct sk_buff *dev_alloc_skb(unsigned int length);
extern struct sk_buff *skb_copy(const struct sk_buff *skb, int gfp_mask);
extern struct sk_buff *skb_clone(struct sk_buff *skb, int gfp_mask);

#if defined(CONFIG_RTL_VLAN_SUPPORT)
static inline int skb_cow(struct sk_buff *skb, unsigned int headroom)
{
	return -1;
}
#endif

#endif /* __SKBUFF_H__ */
