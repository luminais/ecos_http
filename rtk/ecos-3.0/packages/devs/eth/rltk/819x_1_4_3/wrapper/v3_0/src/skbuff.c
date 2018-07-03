 /******************************************************************************
  *
  * Name: skbuff.c - wrapper of Linux skb buffer module
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

/* System include files */
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include <sys/mbuf.h>

#ifdef CONFIG_RTL_VLAN_SUPPORT
extern int rtl_vlan_support_enable;
#endif

//#define DEBUG

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH
extern int is_rtl865x_eth_priv_buf(unsigned char *head);
extern void free_rtl865x_eth_priv_buf(unsigned char *head);
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
extern int is_rtl8190_priv_buf(unsigned char *head);
extern void free_rtl8190_priv_buf(unsigned char *head);
#endif

struct skb_buf {
	struct list_head list;
	struct sk_buff skb;
};

static struct skb_buf skb_pool[MAX_LOCAL_SKB_NUM];
__DMEM_SECTION__ static struct list_head wrapper_skbbuf_list;
__DMEM_SECTION__ int skbbuf_used_num;
__DMEM_SECTION__ static int max_skbbuf_used_num;
int skb_malloc_fail_num;
int cluster_malloc_fail_num;

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
#define MAX_SKB_BUF_SIZE	2048
#define MAX_SKB_BUF_NUM		512

struct skb_data {
	struct list_head list;
	unsigned char buf[MAX_SKB_BUF_SIZE];
};

static struct skb_data skb_data_pool[MAX_SKB_BUF_NUM];
static struct list_head skbdata_list;
int skbdata_used_num;
static int max_skbdata_used_num;
#endif

__IMEM_SECTION__ 
static __inline__ unsigned char *get_buf_from_poll(struct list_head *phead, int *count)	
{
	struct sk_buff *skb;
	struct list_head *plist;

	if (list_empty(phead)) 
		return NULL;	

	plist = phead->next;
	list_del_init(plist);
	
	skb = (struct sk_buff *)((unsigned int)plist + sizeof(struct list_head));

	*count = *count + 1;	

	return (unsigned char *)skb;
}

__IMEM_SECTION__ 
static void release_buf_to_poll(unsigned char *buf, struct list_head *phead, int *count)
{
	struct list_head *plist;
	plist = (struct list_head *)(((unsigned int)buf) - sizeof(struct list_head));
		
	list_add_tail(plist, phead);

	*count = *count -1;	
}

void init_skb_pool(void)
{
	int i;

	memset(skb_pool, '\0', sizeof(skb_pool));
	INIT_LIST_HEAD(&wrapper_skbbuf_list);
	
	for (i=0; i<MAX_LOCAL_SKB_NUM; i++) {
		INIT_LIST_HEAD(&skb_pool[i].list);
		list_add_tail(&skb_pool[i].list, &wrapper_skbbuf_list);
	}
	skbbuf_used_num = 0;	
	max_skbbuf_used_num = 0;
	skb_malloc_fail_num=0;
	cluster_malloc_fail_num=0;
}

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
void init_skb_data_pool(void)
{
	int i;

	memset(skb_data_pool, '\0', sizeof(skb_data_pool));
	INIT_LIST_HEAD(&skbdata_list);
	
	for (i=0; i<MAX_SKB_BUF_NUM; i++) {		
		INIT_LIST_HEAD(&skb_data_pool[i].list);
		list_add_tail(&skb_data_pool[i].list, &skbdata_list);
	}
	skbdata_used_num = 0;	
	max_skbdata_used_num = 0;
}
#endif

#ifdef TX_SCATTER
struct sk_buff *alloc_skb_buf(unsigned char *buf, int len)
{
	struct sk_buff *skb;
	u32 x;

	save_and_cli(x);
	skb = (struct sk_buff *)get_buf_from_poll(&wrapper_skbbuf_list, &skbbuf_used_num);
	restore_flags(x);		

	if (skb == NULL) {
		skb_malloc_fail_num++;
		return NULL;
	}
	
	memset(skb, '\0', sizeof(*skb));
	
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	#if 0
	skb->head = buf-6;	/*reserve VLAN_HLEN + 2*/
	skb->data = buf;
	skb->tail = buf;
	skb->end = buf + len;
	#else
	if(rtl_vlan_support_enable){
		skb_assign_buf(skb, buf-(VLAN_HLEN+2), len+(VLAN_HLEN+2));
		skb_reserve(skb, VLAN_HLEN+2);
	}else
	#endif
#endif
	skb_assign_buf(skb, buf, len);

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	skb->ref = (unsigned char *)&skb->data_ref;
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
#endif

	skb->len = len;
	return skb;
}
#endif

__IMEM_SECTION__
struct sk_buff *alloc_skb(int size)
{
	struct sk_buff *skb;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	caddr_t data;
#else
	unsigned char *data;
#endif
	u32 x;

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (size > MCLBYTES) {
		diag_printf("size %d > MCLBYTES\n", size);
		return NULL;
	}
#endif

	save_and_cli(x);
	skb = (struct sk_buff *)get_buf_from_poll(&wrapper_skbbuf_list, &skbbuf_used_num);
	restore_flags(x);		
	
	if (skb == NULL) {
		skb_malloc_fail_num++;
		return NULL;
	}
	
	memset(skb, '\0', sizeof(*skb));

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	MCLALLOC(data, M_DONTWAIT);
	//diag_printf("data=%p\n", data);
#else
	save_and_cli(x);
	data = (unsigned char **)get_buf_from_poll(&skbdata_list, &skbdata_used_num);
	restore_flags(x);
#endif
	if (data == NULL) {
		//diag_printf("MCLALLOC fail\n");
		cluster_malloc_fail_num++;
		save_and_cli(x);
		release_buf_to_poll((unsigned char *)skb, &wrapper_skbbuf_list, &skbbuf_used_num);
		restore_flags(x);
		return NULL;
	}

	if (skbbuf_used_num > max_skbbuf_used_num) {		
		max_skbbuf_used_num = skbbuf_used_num;
		//diag_printf("		max_skbbuf_used_num = %d\n", max_skbbuf_used_num);
	}
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (skbdata_used_num > max_skbdata_used_num) {
		max_skbdata_used_num = skbdata_used_num;
		//diag_printf("		max_skbdata_used_num = %d\n", max_skbdata_used_num);		
	}
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	size = MCLBYTES;
#else
	size = SKB_DATA_ALIGN(size + 128);
#endif
	skb_assign_buf(skb, (unsigned char *)data, size);

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
 	skb->ref =  (unsigned char *)&skb->data_ref;
	atomic_set(&(skb_shinfo(skb)->dataref), 1);
#endif
#if defined(CONFIG_RTL_VLAN_SUPPORT)
	skb->tag.v = 0;
#endif
#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_init_wanorlan(&skb->wanorlan);
#endif

#ifdef CONFIG_RTL_DELAY_REFILL
	skb->priv = NULL;
#endif
	return skb;
}

__IMEM_SECTION__
struct sk_buff *dev_alloc_skb(unsigned int length)
{
	struct sk_buff *skb;
/*for ecos, can reserve less. HF 20121112*/
#ifdef __ECOS
	skb = alloc_skb(length+32);
	if (skb)
		skb_reserve(skb, 32);
#else
	skb = alloc_skb(length+32);
	if (skb)
		skb_reserve(skb, 32);
#endif
	return skb;
}

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size)
{
	struct sk_buff *skb;
	u32 x;

	save_and_cli(x);
	skb = (struct sk_buff *)get_buf_from_poll(&wrapper_skbbuf_list, &skbbuf_used_num);
	restore_flags(x);		
	
	if (skb == NULL) {
		skb_malloc_fail_num++;
		return NULL;
	}

	if (skbbuf_used_num > max_skbbuf_used_num) {		
		max_skbbuf_used_num = skbbuf_used_num;
		//diag_printf("		max_skbbuf_used_num = %d\n", max_skbbuf_used_num);		
	}
	
	memset(skb, '\0', sizeof(*skb));
	size = SKB_DATA_ALIGN(size + 128);

	skb_assign_buf(skb, data, size);

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
  	skb->ref =  (unsigned char *)&skb->data_ref;
 	atomic_set(&(skb_shinfo(skb)->dataref), 1);
#endif
#if defined(CONFIG_RTL_VLAN_SUPPORT)
	skb->tag.v = 0;
#endif

	/* Set up other state */
  	skb_reserve(skb, 128);
	return skb;
}
#endif

#ifdef CONFIG_RTL_DELAY_REFILL
/*this delay refill will alloc a new skb to refill rx ring. since skb->data may already been freed */
void  __attribute__((mips16)) rtk_delay_refill(struct sk_buff *skb)
{
	int ret=0;
	if(skb->priv ==NULL)
		ret=rtk_eth_delay_refill(skb);
	else
		ret=rtk_wifi_delay_refill(skb);
	return ret;
}	
#endif

void kfree_skb(struct sk_buff *skb)
{
	u32 x;

	save_and_cli(x);
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (!skb->cloned ||
	 	(skb->ref && atomic_dec_and_test(&(skb_shinfo(skb)->dataref))))
#endif
	{
#ifdef TX_SCATTER
		if (skb->key)
			goto free_skb;
#endif
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH
		if (is_rtl865x_eth_priv_buf(skb->head)) {
			free_rtl865x_eth_priv_buf(skb->head);
			goto free_skb;
		} 
#endif
#endif

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
		if (is_rtl8190_priv_buf(skb->head)) {
			free_rtl8190_priv_buf(skb->head);
			goto free_skb;
		}		
#endif
#endif
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		if (skb->head) {
			//diag_printf("free=%p key=%x\n", skb->head, skb->key);
			MCLFREE(skb->head);
		}
#else
		release_buf_to_poll(skb->head, &skbdata_list, &skbdata_used_num);
#endif
	}

free_skb:
#ifdef CONFIG_RTL_DELAY_REFILL
	rtk_delay_refill(skb);
#endif
	release_buf_to_poll((unsigned char *)skb, &wrapper_skbbuf_list, &skbbuf_used_num);
	restore_flags(x);
}

void kfree_skb_chk_key(struct sk_buff *skb, struct net_device *root_dev)
{
#ifdef TX_SCATTER
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (skb->key && skb->dev)
#else
	if (skb->key && skb->dev && !skb_cloned(skb))
#endif
	{
#ifdef TX_PKT_FREE_QUEUE
		if(netif_running(root_dev))
			wrapper_que_free_tx_pkt(root_dev, skb);
		else {
			struct eth_drv_sc *sc = ((Rltk819x_t *)skb->dev->info)->sc;			
			(sc->funs->eth_drv->tx_done)(sc, skb->key, 0);			
			kfree_skb(skb);
		}	
		return;
#else
		struct eth_drv_sc *sc = ((Rltk819x_t *)skb->dev->info)->sc;
		(sc->funs->eth_drv->tx_done)(sc, skb->key, 0);
		skb->head = NULL;
#endif
	}
#endif	
	kfree_skb(skb);
}

struct sk_buff *skb_copy(const struct sk_buff *skb, int gfp_mask)
{
        struct sk_buff *n;
#ifdef TX_SCATTER
	int i;
#else
        int headerlen = skb->data-skb->head;
#endif
	
        /*
         *      Allocate the copy buffer
         */
#ifdef TX_SCATTER
	n = dev_alloc_skb(skb->total_len);
	if (n == NULL)
		return NULL;
#else
        n = alloc_skb(skb->end - skb->head + headerlen);
        if (n == NULL)
                return NULL;
	/* Set the data pointer */
	skb_reserve(n,headerlen);
#endif

#ifdef TX_SCATTER
	if (skb->list_num > 0)
	{
		for (i = 0; i<skb->list_num; i++) {
			memcpy(n->tail, skb->list_buf[i].buf, skb->list_buf[i].len);
			skb_put(n, skb->list_buf[i].len);
		}
	}
	else
#endif
	{
	        /* Set the tail pointer and length */
	        skb_put(n,skb->len);
	        //n->csum = skb->csum;
	        //n->ip_summed = skb->ip_summed;
	
	        //if (skb_copy_bits(skb, -headerlen, n->head, headerlen+skb->len))
	        //        BUG();
		memcpy(n->data, skb->data, n->len);
		n->len = skb->len;
	}
	n->dev = skb->dev;
	#if defined(CONFIG_RTL_VLAN_SUPPORT)
	n->tag.v = skb->tag.v;
    #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    n->taged = skb->taged;
    n->flag_src = skb->flag_src;
    n->forward_rule = skb->forward_rule;
    n->index = skb->index;
    #endif
	#endif
    #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_set_wanorlan(&n->wanorlan,skb->wanorlan);
    #endif
    #ifdef CONFIG_RTL_DELAY_REFILL
    n->priv = skb->priv;
    #endif
	copy_skb_header(n, skb);
	return n;
}


/**
 *      skb_clone       -       duplicate an sk_buff
 *      @skb: buffer to clone
 *      @gfp_mask: allocation priority
 *
 *      Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *      copies share the same packet data but not structure. The new
 *      buffer has a reference count of 1. If the allocation fails the 
 *      function returns %NULL otherwise the new buffer is returned.
 *      
 *      If this function is called from an interrupt gfp_mask() must be
 *      %GFP_ATOMIC.
 */
struct sk_buff *skb_clone(struct sk_buff *skb, int gfp_mask)
{
	struct sk_buff *n;
	u32 x;


#ifdef TX_SCATTER
	if (skb->list_num > 0) {
		return skb_copy(skb, gfp_mask);
	}
#endif

	save_and_cli(x);
	n = (struct sk_buff *)get_buf_from_poll(&wrapper_skbbuf_list, &skbbuf_used_num);
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	if (n) {
		++mclrefcnt[mtocl(skb->head)];
	}
#endif
	restore_flags(x);
	
	if (n == NULL) {
		skb_malloc_fail_num++;
		return NULL;
	}
	
	memset(n, '\0', sizeof(*n));
	
#define C(x) n->x = skb->x

//        n->next = n->prev = NULL;
        n->list = NULL;
//        n->sk = NULL;
//        C(stamp);
        C(dev);
//        C(h);
//        C(nh);
//        C(mac);
//        C(dst);
//        dst_clone(n->dst);
        memcpy(n->cb, skb->cb, sizeof(skb->cb));
        C(len);
//        C(data_len);
//        C(csum);
        n->cloned = 1;
//        C(pkt_type);
//        C(ip_summed);
//        C(priority);
//        atomic_set(&n->users, 1);
//        C(protocol);
//        C(security);
//        C(truesize);
        C(head);
        C(data);
        C(tail);
        C(end);
//        n->destructor = NULL;
#ifdef CONFIG_NETFILTER
        C(nfmark);
        C(nfcache);
        C(nfct);
#ifdef CONFIG_NETFILTER_DEBUG
        C(nf_debug);
#endif
#endif /*CONFIG_NETFILTER*/
#if defined(CONFIG_HIPPI)
        C(private);
#endif
#ifdef CONFIG_NET_SCHED
        C(tc_index);
#endif

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
        atomic_inc(&(skb_shinfo(skb)->dataref));
#endif

        skb->cloned = 1;
#ifdef CONFIG_NETFILTER
        nf_conntrack_get(skb->nfct);
#endif
#ifdef CONFIG_RTK_VOIP_VLAN_ID
        C(rx_vlan);
        C(rx_wlan);
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)||defined(CONFIG_RTK_VLAN_SUPPORT)
        C(tag.v);
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
        n->flag_src = skb->flag_src;
        n->forward_rule = skb->forward_rule;
        n->taged = skb->taged;
        n->index = skb->index;
#endif
#endif

#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
        rtl_set_wanorlan(&n->wanorlan,skb->wanorlan);
#endif

#ifdef CONFIG_RTL_DELAY_REFILL
	C(priv);
#endif
#ifdef CONFIG_NET_RADIO
        n->__unused = 0;
#endif

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	C(ref);
	C(key);
#endif

        return n;
}

unsigned char * skb_pull(struct sk_buff *skb, unsigned int len)
{	
	if (len > skb->len)
		return NULL;
	return __skb_pull(skb,len);
}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
int skb_cloned(struct sk_buff *skb)
{
	char refcnt;
	unsigned long flags;
	struct mbuf *m0;
	extern char *mclrefcnt;
	
	m0 = (struct mbuf *)skb->key;
	HAL_DISABLE_INTERRUPTS(flags);
	if (m0) {
		if (m0->m_flags & M_EXT)
			refcnt = mclrefcnt[mtocl(skb->head)];
		else
			refcnt = 1;
	}
	else {
		refcnt = mclrefcnt[mtocl(skb->head)];
	}
	HAL_RESTORE_INTERRUPTS(flags);
	//return (skb->cloned && (refcnt != 1));
	return (refcnt != 1);
}
#endif

void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

void dump_skb_info(void)
{
	diag_printf("total=%d\n", MAX_LOCAL_SKB_NUM);
	diag_printf("max_skbbuf_used_num=%d\n", max_skbbuf_used_num);
	diag_printf("skbbuf_used_num=%d\n", skbbuf_used_num);
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	diag_printf("skbdata_used_num=%d\n", skbdata_used_num);
#endif
	diag_printf("skb_malloc_fail_num=%d\n", skb_malloc_fail_num);
	diag_printf("cluster_malloc_fail_num=%d\n", cluster_malloc_fail_num);
}

void test_skb(void)
{
	int num;
	struct sk_buff_head skb_queue;
	struct sk_buff *skb;
	
	num = 0;
	skb_queue_head_init(&skb_queue);
	while (1) {
		skb = alloc_skb(1000);
		if (skb == NULL)
			break;
		num++;
		skb_queue_tail(&skb_queue, skb);
	}
	diag_printf("Alloc:num=%d skb_queue.qlen=%d\n", num, skb_queue.qlen);
	diag_printf("available skb: %d\n", num);	
	while (skb_queue.qlen > 0) {
		skb = skb_dequeue(&skb_queue);
		if (skb == NULL)
			break;
		num--;
		kfree_skb(skb);
	}
	diag_printf("Free: num=%d skb_queue.qlen=%d\n", num, skb_queue.qlen);
}
