 /******************************************************************************
  *
  * Name: sys-support.h - System type support for Linux
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

#ifndef __SYS_SUPPORT_H__
#define __SYS_SUPPORT_H__

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <sys/param.h>
#include <sys/malloc.h>
#include <string.h>

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
#define CONFIG_RTL865X_ETH_PRIV_SKB
#endif

//#define ISR_DIRECT	/* process event in ISR directly */
#define TX_SCATTER
//#define TX_ISR_DIRECT
#define TX_PKT_FREE_QUEUE

#ifndef REVR
#define REVR						0xB8000000
#define RTL8196C_REVISION_A	0x80000001
#define RTL8196C_REVISION_B	0x80000002
#endif

#ifdef CYGSEM_HAL_IMEM_SUPPORT
#define __IMEM_SECTION__	__attribute__ ((section(".iram-gen")))
#else
#define __IMEM_SECTION__
#endif

#ifdef CYGSEM_HAL_DMEM_SUPPORT
#define __DMEM_SECTION__	__attribute__ ((section(".dram-gen")))
#else
#define __DMEM_SECTION__
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_USE_MIPS16
#define __MIPS16__		__attribute__ ((mips16))
#else
#define __MIPS16__
#endif

#define	EIO			 5	/* I/O error */
#define	ENOMEM		12	/* Out of memory */
#define	EFAULT		14	/* Bad address */
#define	HZ 			100

#define	RLTK_PAGE_SHIFT	12
#define	RLTK_PAGE_SIZE	(1L << RLTK_PAGE_SHIFT)

#ifdef PAGE_SIZE
#if PAGE_SIZE != RLTK_PAGE_SIZE
	#undef PAGE_SIZE
	#define PAGE_SIZE RLTK_PAGE_SIZE
#endif
#else
	#define PAGE_SIZE	RLTK_PAGE_SIZE
#endif

#define SMP_CACHE_BYTES	16
#define SKB_DATA_ALIGN(X)	(((X) + (SMP_CACHE_BYTES-1)) & ~(SMP_CACHE_BYTES-1))

#if 1 //def CONFIG_RTL_CLIENT_MODE_SUPPORT  //mark_ecos
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#define ETH_P_IPX       0x8137          /* IPX over DIX                 */
#define ETH_FRAME_LEN   1514         /* Max. octets in frame sans FCS */ 
#define ETH_P_PPP_DISC  0x8863          /* PPPoE discovery messages     */
#define ETH_P_PPP_SES   0x8864          /* PPPoE session messages       */
#define PADI_CODE       0x09
#define PADO_CODE       0x07
#define PADR_CODE       0x19
#define PADS_CODE       0x65
#define PTT_RELAY_SID   __constant_htons(0x0110)
#define ETH_P_8021Q     0x8100          /* 802.1Q VLAN Extended Header  */
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/

#endif

/* 
 *
 */
#ifndef NULL
#define NULL				(void *) 0
#endif

#define KERN_ERR
#define KERN_INFO
#define SET_MODULE_OWNER(some_struct) 	do { } while (0)
#define __devinitdata
#define GFP_KERNEL		1
#define GFP_ATOMIC		1
#define SA_INTERRUPT	0
#define ETH_ALEN		6		/* Octets in one ethernet addr   */
#define ETH_HLEN		14		/* Total octets in header.       */
#define ETH_P_ARP		0x0806	/* Address Resolution packet    */

#define __init
#define __exit
#define __devinit
#define __devexit

/* Platform constant definition */
#define PCI_DMA_TODEVICE		1
#define PCI_DMA_FROMDEVICE	2
#define PCI_VENDOR_ID_REALTEK	0x10ec
#define PCIE0_D_CFG0    			0xB8B10000
#define PCIE0_D_MEM				0xB9000000
#define PCIE_IRQ        			BSP_PCIE_IRQ

#define readb(addr)				(*(volatile unsigned char *)(addr))
#define readw(addr)				(*(volatile unsigned short *)(addr))
#define readl(addr)				(*(volatile unsigned long *)(addr))
#define writeb(b,addr)			((*(volatile unsigned char *)(addr)) = (b))
#define writew(b,addr)			((*(volatile unsigned short *)(addr)) = (b))
#define writel(b,addr)				((*(volatile unsigned int *)(addr)) = (b))

#define le16_to_cpu(x) \
         ((u16)((((u16)(x) & 0x00FFU) << 8) | \
                   (((u16)(x) & 0xFF00U) >> 8)))

#define le32_to_cpu(x) \
         ((u32)((((u32)(x) & 0x000000FFU) << 24) | \
                   (((u32)(x) & 0x0000FF00U) <<  8) | \
                   (((u32)(x) & 0x00FF0000U) >>  8) | \
                   (((u32)(x) & 0xFF000000U) >> 24)))

#define le64_to_cpu(x) \
         ((u64)((((u64)(x) & 0x00000000000000FFULL) << 56) | \
                   (((u64)(x) & 0x000000000000FF00ULL) << 40) | \
                   (((u64)(x) & 0x0000000000FF0000ULL) << 24) | \
                   (((u64)(x) & 0x00000000FF000000ULL) <<  8) | \
                   (((u64)(x) & 0x000000FF00000000ULL) >>  8) | \
                   (((u64)(x) & 0x0000FF0000000000ULL) >> 24) | \
                   (((u64)(x) & 0x00FF000000000000ULL) >> 40) | \
                   (((u64)(x) & 0xFF00000000000000ULL) >> 56))) 

#define cpu_to_le16(x)	le16_to_cpu(x)
#define cpu_to_le32(x)	le32_to_cpu(x)
#define cpu_to_le64(x)	le64_to_cpu(x)
#define cpu_to_be16(x)	(x)
#define cpu_to_be32(x)	(x)
#define _constant_cpu_to_le16(x)	cpu_to_le16(x)
#define __constant_htons(x) ((u16)(x))
#define __constant_cpu_to_le16(x)	cpu_to_le16(x)
/*
#define ntohs(x)			((u16)(x))
#define ntoh(x)			((u16)(x))
#define ntohl(x)			((u32)(x))
#define htons(x)			((u16)(x))
#define htonl(x)			((u32)(x))
*/
#define BUG()			do { } while (1)

/* Return count in buffer.  */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

 /* Return space available, 0..size-1.  We always leave one free char
    as a completely full buffer has head == tail, which is the same as
    empty.  */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

#ifdef CONFIG_PRINTK
#define printk				diag_printf
#else
#define printk(fmt, args...) 			do { } while (0)
#endif

#define spin_lock_init(lock)			do { } while (0)
#define spin_lock(x)					do { } while (0)
#define spin_unlock(x)				do { } while (0)
#define restore_flags(x)			HAL_RESTORE_INTERRUPTS(x)
#define save_and_cli(x)			HAL_DISABLE_INTERRUPTS(x)
//extern void restore_flags(unsigned int x);
//extern void save_and_cli(unsigned int x);
#define spin_lock_irqsave(lock, flags)	save_and_cli(flags)
#define spin_unlock_irqrestore(l, f)	restore_flags(f)

#define cli() { \
	u32 x=0; \
	save_and_cli(x); \
}

#define unregister_netdev(dev)		do { } while (0)
#define iounmap(regs)				do { } while (0)
#define netif_start_queue(dev)		do { } while (0)
#define netif_queue_stopped(dev)		(0)
#define netif_wake_queue(dev)		do { } while (0)
#define netif_stop_queue(dev)		do { } while (0)
#define free_irq(rq, dev)				do { } while (0)
#define SMP_LOCK(flag)				do { } while (0)
#define SMP_UNLOCK(flag)				do { } while (0)

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
#define kmalloc(size, flag)		cyg_net_malloc((u_long)size, M_DEVBUF, M_NOWAIT)
#define kfree(buf)			cyg_net_free((caddr_t)(buf), M_DEVBUF)
#else
#define kmalloc(size, flag)		alloc_local(size)
//#define kfree(buf)			do { } while (0)
static inline void kfree(void *new)
{
	char tmp[200];
	diag_sprintf(tmp, "[%s %s, %d] Should not go here!!\n", __FILE__, __FUNCTION__, __LINE__);
	CYG_ASSERT(0, tmp);
}
#endif

#define mdelay(t)						HAL_DELAY_US(1000*t)
#define __delay(t)					mdelay(t/1000)
#define ASSERT(_bool_)				CYG_ASSERTC(_bool_)
#define __udelay(t,v)					HAL_DELAY_US(t)

#define _dma_cache_wback_inv(addr, size) HAL_DCACHE_FLUSH(addr, size)
#define dma_cache_wback_inv(addr, size)	_dma_cache_wback_inv(addr, size)

#ifdef ISR_DIRECT
#define netif_rx(skb, dev)				wrapper_que_up(skb, dev)
#else
#define netif_rx(skb)					wrapper_up(skb)
#endif
#define panic_printk					diag_printf
#define sprintf						diag_sprintf
#define copy_from_user(a,b,c)		(memcpy(a,b,c)==a ? 0 : c )
#define copy_to_user(a,b,c)		(memcpy(a,b,c)==a ? 0 : c )

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})


/* 
 *	Type definition
 */
 
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef long unsigned int LUINT;
typedef struct { volatile int counter; } atomic_t;
typedef unsigned long long u64;
typedef unsigned short __be16;

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

/*
#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)
*/

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __list_add(struct list_head * new,
	struct list_head * prev,
	struct list_head * next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void list_add(struct list_head *new, struct list_head *head);
/*static __inline__ void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}*/

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail(struct list_head *new, struct list_head *head);
/*static __inline__ void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}
*/

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __list_del(struct list_head * prev,
				  struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
static __inline__ void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static __inline__ void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static __inline__ int list_empty(struct list_head *head)
{
	return head->next == head;
}

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static __inline__ void list_splice(struct list_head *list, struct list_head *head)
{
	struct list_head *first = list->next;

	if (first != list) {
		struct list_head *last = list->prev;
		struct list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))


static __inline__ void *memscan(void * addr, int c, int size)
{
	unsigned char * p = (unsigned char *) addr;
	unsigned char * e = p + size;

	while (p != e) {
		if (*p == c)
			return (void *) p;
			p++;
	}
	return (void *) p;
}

#define IW_PRIV_TYPE_NONE	0x0000
#define SIOCGIWNAME		0x8B01	/* get name == wireless protocol */
#define SIOCDEVPRIVATE		0x89F0	/* to 89FF */
#define SIOCGIWPRIV			0x8B0D	/* get private ioctl interface info */
#define IW_PRIV_TYPE_BYTE	0x1000	/* Char as number */
#define IW_PRIV_TYPE_CHAR	0x2000	/* Char as character */
#define VERIFY_READ			0
#define VERIFY_WRITE   		1
#define IFNAMSIZ       		16


/*
  * Memory segments (32bit kernel mode addresses)
  */
 #define KUSEG                   0x00000000
 #define KSEG0                   0x80000000
 #define KSEG1                   0xa0000000
 #define KSEG2                   0xc0000000
 #define KSEG3                   0xe0000000
  #define K0BASE                 KSEG0
 
 /*
   * Returns the kernel segment base of a given address
   */
 #define KSEGX(a)                 ((((unsigned int)a)) & 0xe0000000)
 #define PHYSADDR(a)          ((((unsigned int)a)) & 0x1fffffff)
 #define KSEG0ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG0)
 #define KSEG1ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG1)
 #define KSEG2ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG2)
 #define KSEG3ADDR(a)        (((((unsigned int)a)) & 0x1fffffff) | KSEG3)
 

/*
  * Change virtual addresses to physical addresses and vv.
  * These are trivial on the 1:1 Linux/MIPS mapping
  */

static inline unsigned long virt_to_phys(volatile void * address)
{
       return PHYSADDR(address);
}
 
static inline void * phys_to_virt(unsigned long address)
{
       return (void *)KSEG0ADDR(address);
}
 
/*
  * IO bus memory addresses are also 1:1 with the physical address
  */
static inline unsigned long virt_to_bus(volatile void * address)
{
        return PHYSADDR(address);
}
 
static inline void * bus_to_virt(unsigned long address)
{
        return (void *)KSEG0ADDR(address);
}

#ifndef CYGPKG_KERNEL
static inline void get_random_bytes(void *buf, int len)
{
	char *seed="this is random data seed";
	unsigned char *data, *dst;
	static int call=2;
	int i, j;

	data = (unsigned char *)seed;
	for (i=0; i<call; i++) {
		dst = (unsigned char *)buf;
		for (j=0; j<len; j++) {
			if (*data == 0)
				data = seed;
			*dst = *dst ^ *data;
			dst++;
			data++;
		}		
	}

	if (call++ >= 10)
		call = 2;
}
#endif


/*
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically reads the value of @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_read(v)  ((v)->counter)

/*
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_set(v,i) ((v)->counter = (i))


/*
 * The MIPS I implementation is only atomic with respect to
 * interrupts.  R3000 based multiprocessor machines are rare anyway ...
 *
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v.  Note that the guaranteed useful range
 * of an atomic_t is only 24 bits.
 */
extern __inline__ void atomic_add(int i, atomic_t * v)
{
	int flags;

	save_and_cli(flags);
	v->counter += i;
	restore_flags(flags);
}

/*
 * atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
extern __inline__ void atomic_sub(int i, atomic_t * v)
{
	int flags;

	save_and_cli(flags);
	v->counter -= i;
	restore_flags(flags);
}

extern __inline__ int atomic_add_return(int i, atomic_t * v)
{
	int temp, flags;

	save_and_cli(flags);
	temp = v->counter;
	temp += i;
	v->counter = temp;
	restore_flags(flags);

	return temp;
}

extern __inline__ int atomic_sub_return(int i, atomic_t * v)
{
	int temp, flags;

	save_and_cli(flags);
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	restore_flags(flags);

	return temp;
}

/*
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically increments @v by 1.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_inc(v) atomic_add(1,(v))

/*
 * atomic_dec - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_dec(v) atomic_sub(1,(v))

/*
 * atomic_dec_and_test - decrement by 1 and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)


static inline u_int32_t rtk_get_unaligned_u32(char *p)
{
	if ((u_int32_t)p & 0x03) { //not 4-byte alignment
		u_int32_t val;
		char *pval = (char *)&val;

		pval[0] = p[0];
		pval[1] = p[1];
		pval[2] = p[2];
		pval[3] = p[3];
		return val;
	}
	else {
		return *((u_int32_t *)p);
	}
}

static inline void rtk_put_unaligned_u32(u_int32_t val, char *p)
{
	if ((u_int32_t)p & 0x03) { //not 4-byte alignment
		char *pval = (char *)&val;

//		pval[0] = p[0];
//		pval[1] = p[1];
//		pval[2] = p[2];
//		pval[3] = p[3];

		p[0] = pval[0];
		p[1] = pval[1];
		p[2] = pval[2];
		p[3] = pval[3];
	}
	else {
		*((u_int32_t *)p) = val;
	}
}

/*
 *	External functions
 */

extern void *rtl865x_probe(int ethno);
#endif /* __SYS_SUPPORT_H__ */
