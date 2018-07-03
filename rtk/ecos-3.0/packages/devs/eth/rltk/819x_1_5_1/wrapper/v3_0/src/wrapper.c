 /******************************************************************************
  *
  * Name: wrapper.c - Wrapper module for Linux network device driver
  *       $Revision: 1.1.1.1 $
  *
  *****************************************************************************/

/* System include files */

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#ifdef CYGPKG_IO_ETH_DRIVERS
#include <pkgconf/io_eth_drivers.h>
#endif
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <sys/mbuf.h>

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
extern int rtl_vlan_support_enable;
#endif

/* Constant declarations */

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
/* max local memory pool size */
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
#define MAX_POOL_SIZE	(400*1024)
#else
#define MAX_POOL_SIZE	(236*1024)
#endif

/* Local driver function declarations */

static char memory_pool[MAX_POOL_SIZE];
static int memory_used; /* used size count */
#endif

static int eth_idx = 0;

#ifndef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
static void init_local_pool(void)
{
	memset(memory_pool, '\0', sizeof(memory_pool));
	memory_used = 0;
}

/*
 *  Allocate a memory from local pool
 */
void *alloc_local(int size)
{
	void *tmp;
	unsigned int offset;

	offset = RLTK_PAGE_SIZE - (((unsigned int)&memory_pool[memory_used]) & 0xfff);

	if ((size + memory_used + offset) > MAX_POOL_SIZE) {
		DBG_ERR("%s: local buffer pool exhausted(%d)!\n", __FUNCTION__, (size + memory_used + offset));
		return NULL;
	}

	tmp = &memory_pool[memory_used+offset];
	memory_used += (size + offset);

	DBG_INFO("%s: size=%d, memory_used=%d, ptr=0x%lx\n", __FUNCTION__, size, memory_used, (long)tmp);
	//DBG_ERR("%s: size=%d, memory_used=%d, ptr=0x%lx\n", __FUNCTION__, size, memory_used, (long)tmp);
	return tmp;
}
#endif

#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB
/*fast bridge*/
#define	mix(a,b,c) \
	do {						\
		a -= b; a -= c; a ^= (c >> 13);		\
		b -= c; b -= a; b ^= (a << 8);		\
		c -= a; c -= b; c ^= (b >> 13);		\
		a -= b; a -= c; a ^= (c >> 12);		\
		b -= c; b -= a; b ^= (a << 16);		\
		c -= a; c -= b; c ^= (b >> 5);		\
		a -= b; a -= c; a ^= (c >> 3);		\
		b -= c; b -= a; b ^= (a << 10);		\
		c -= a; c -= b; c ^= (b >> 15);		\
	} while(0)

#define FAST_BRIDGE_RTABLE_SIZE   64
#define FAST_BRIDGE_RTABLE_MASK (FAST_BRIDGE_RTABLE_SIZE-1)

int mac_hash(unsigned char *da)
{
	unsigned int a = 0x9e3779b9, b = 0x9e3779b9, c = 0xdeadbeef;

	b += da[5] << 8;
	b += da[4];
	a += da[3] << 24;
	a += da[2] << 16;
	a += da[1] << 8;
	a += da[0];

	mix(a, b, c);
	return (c & FAST_BRIDGE_RTABLE_MASK);
}

struct fdb_simple {
	struct net_device *dev;
	unsigned char mac_addr[6];
}  fdb_array[FAST_BRIDGE_RTABLE_SIZE];

struct net_device * getDevByMac(unsigned char * mac)
{
	int index;
	index=mac_hash(mac);
	if(0==memcmp(mac,fdb_array[index].mac_addr,6))
	{
		return fdb_array[index].dev;
	}
	return NULL;
}

void dump_fst_fdb()
{
	int i;
	diag_printf("dump fst fdb\n");
	for(i=0;i<FAST_BRIDGE_RTABLE_SIZE;i++)
	{
		diag_printf("%d:\n",i);		
		diag_printf("mac: %02x%02x%02x%02x%02x%02x\n",fdb_array[i].mac_addr[0],
			fdb_array[i].mac_addr[1],fdb_array[i].mac_addr[2],fdb_array[i].mac_addr[3],
			fdb_array[i].mac_addr[4],fdb_array[i].mac_addr[5]);
		diag_printf("dev :%p\n",fdb_array[i].dev);
	}
}
void addfdbentry(unsigned char *mac,struct net_device *dev)
{
	int index;
	index=mac_hash(mac);
	/*
	diag_printf("%s %d\n",__FUNCTION__,__LINE__);
	diag_printf("index %d mac: %02x%02x%02x%02x%02x%02x\n",index,mac[0],
			mac[1],mac[2],mac[3],
			mac[4],mac[5]);
	*/
	fdb_array[index].dev=dev;
	memcpy(fdb_array[index].mac_addr,mac,6);
}
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
extern int passThruStatusWlan;
#if defined(CONFIG_RTL_819X_SWCORE)
extern int old_passThru_flag;
extern int rtl_isEthPassthruFrame(unsigned char *data);
#endif
#if defined(RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0)
extern int rtl_isWlanPassthruFrame(unsigned char *data);
#endif

int rtl_CheckPassthruFrame(unsigned char* data)
{
	int	ret;

	ret = FAILED;
/*
	#if defined(CONFIG_RTL_819X_SWCORE)
	if (passThruStatusWlan||old_passThru_flag)
	#else
	if (passThruStatusWlan)
	#endif
*/
	{
		#if defined(CONFIG_RTL_819X_SWCORE)
		if(old_passThru_flag){
			ret = rtl_isEthPassthruFrame(data);
		}	
		#endif 
		#if defined(RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0)
		if (passThruStatusWlan){
			ret = rtl_isWlanPassthruFrame(data);
		}	
		#endif
	
	}

	return ret;
}
#endif

#if defined(CONFIG_RTL_DNS_TRAP) || defined(__CONFIG_APCLIENT_DHCPC__)
typedef struct _header {
	unsigned short 	id;
	unsigned short		u;

	unsigned short 	qdcount;
	unsigned short 	ancount;
	unsigned short 	nscount;
	unsigned short 	arcount;
} dnsheader_t;

struct udphdr {
	unsigned short	source;
	unsigned short	dest;
	unsigned short	len;
	unsigned short	check;
};
struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
};

#endif

#if defined(CONFIG_RTL_DNS_TRAP)
static inline struct ethhdr *eth_hdr(const struct mbuf *m)
{
	return (struct ethhdr *)m->m_data;
}
#include "../../../../../../../../../ecos-work/AP/apmib/apmib.h"
unsigned char default_url[80]="ul.rtk.com";
unsigned char domainName[MAX_NAME_LEN];
unsigned char dns_answer[] = { 0xC0, 0x0C, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x03, 0x84, 0x00, 0x04};
static int dnstrap_en = 0;

void enable_dnstrap(int en)
{
	dnstrap_en = en;
}

int get_interface_ip(char *interface_name)
{
	int ipaddr = 0;

#if defined(__ECOS)	
	#include <sys/sockio.h>
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	ipaddr = *(int *)(&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
#else
	struct net_device *dev; 
	struct in_device *in_dev;

	dev = dev_get_by_name(&init_net,"br0"); 
	in_dev = in_dev_get(dev);
#endif

	return ipaddr;
}

inline int br_dns_packet_recap(struct mbuf *m)
{
	struct iphdr *iph;
	struct udphdr *udph;
	dnsheader_t *dns_pkt;
	unsigned char mac[ETH_ALEN];
	unsigned int ip;
	unsigned short port;
	unsigned char *ptr = NULL;
	int ipaddr = 0;

	ipaddr = get_interface_ip("eth0");
	
	iph = m->m_data+14;
	udph = (void *)iph + iph->ihl*4;
	dns_pkt = (void *)udph + sizeof(struct udphdr);
	ptr = (void *)udph + ntohs(udph->len);

#if defined(__ECOS)
  #define PAD_ONE_BYTE
	m->m_len += 16;
  #ifdef PAD_ONE_BYTE
	m->m_len += 1;
  #endif
#else
	skb_put(skb, 16);
#endif

	/* swap mac address */
	memcpy(mac, eth_hdr(m)->h_dest, ETH_ALEN);
	memcpy(eth_hdr(m)->h_dest, eth_hdr(m)->h_source, ETH_ALEN);
	memcpy(eth_hdr(m)->h_source, mac, ETH_ALEN);

	/*swap ip address */
	ip = iph->saddr;
	iph->saddr = iph->daddr;
	iph->daddr = ip;
  #ifdef PAD_ONE_BYTE
	iph->tot_len = htons(ntohs(iph->tot_len)+16+1);
  #else
	iph->tot_len = htons(ntohs(iph->tot_len)+16);
  #endif
	iph->id=0;

	/* swap udp port */
	port = udph->source;
	udph->source = udph->dest;
	udph->dest = port;
  #ifdef PAD_ONE_BYTE
	udph->len = htons(ntohs(udph->len)+16+1);
  #else
	udph->len = htons(ntohs(udph->len)+16);
  #endif
	dns_pkt->u = htons(0x8180);
	dns_pkt->qdcount = htons(0x1);
	dns_pkt->ancount = htons(0x1);
	dns_pkt->nscount = htons(0x0);
	dns_pkt->arcount = htons(0x0);

	/* pad Answers */
	memcpy(ptr, dns_answer, 12);
	memcpy(ptr+12, (unsigned char *)&ipaddr, 4);

	/* ip checksum */
#if defined(__ECOS)
	iph->check = 0;
	iph->check = get_ipsum(iph);
#else
	skb->ip_summed = 0;//CHECKSUM_NONE;
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
#endif

	/* udp checksum */
	udph->check = 0;
#if defined(__ECOS)
	udph->check = get_udpsum(iph, udph);
#else
	udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
					udph->len, IPPROTO_UDP,
					csum_partial((char *)udph,
					             udph->len, 0));
#endif

	return 1;
}
#if 0
static inline unsigned char __tolower(unsigned char c)
{
	if (isupper(c))
		c -= 'A'-'a';
	return c;
}

static void str_to_lower(char *s)
{
	if(s!=NULL)
	{
    	while (*s != '\0') 
		{
        	*s = __tolower(*s);
        	++s;
    	}
	}
}
#endif
inline int br_dns_filter_enter(struct mbuf *m)
{
	struct iphdr *iph;
	struct udphdr *udph;
	unsigned char *url = NULL;
	unsigned char buf[254];
	unsigned char url_buf[254];
	int len = 0, token_len = 0, offset=0, recap=0;

	if (dnstrap_en == 0)
		return 0;
	//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (eth_hdr(m)->h_proto != htons(0x0800))
		return 0;
	

	iph = m->m_data+14;
	//diag_printf("%s:%d iph->ihl=%x\n",__FUNCTION__,__LINE__,iph->ihl);
	udph = (void *)iph + iph->ihl*4;

	if (iph->protocol==IPPROTO_UDP && htons(udph->dest) == (53) && ((iph->frag_off & htons(0x3FFF))==0)) {
		url = (void *)udph + sizeof(struct udphdr) + sizeof(dnsheader_t);
		len = htons(udph->len) - sizeof(struct udphdr) - sizeof(dnsheader_t) - 4;
		
		if(len > 254)
		{
			len=254;
		}
		
		memset(buf, 0, 254);
		memcpy(buf, url, len);
		memset(domainName,'\0',MAX_NAME_LEN);
		apmib_get(MIB_DOMAIN_NAME,(void*)domainName);
		//str_to_lower(domainName);
		
		if (len-strlen(domainName) != 6)
			return 0;

		while (len) {
			token_len = buf[offset];
			if (token_len) {
				buf[offset] = '.';
			}
			token_len +=1;
			len -= token_len;
			offset += token_len;
		}
		//str_to_lower(buf);
		
		memset(url_buf, 0, 254);
		sprintf(url_buf, "%s.%s", domainName, "com");
		if (recap==0 && memcmp(buf+1, url_buf, strlen(url_buf))==0)
			recap = 1;

		memset(url_buf, 0, 254);
		sprintf(url_buf, "%s.%s", domainName, "net");
		if (recap==0 && memcmp(buf+1, url_buf, strlen(url_buf))==0)
			recap = 1;

		if (recap)
			br_dns_packet_recap(m);
		return 1;
	}

	return 0;
}

int is_dns_packet(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct udphdr *udph;

	if (dnstrap_en == 0)
		return 0;
//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	if (((struct ethhdr *)skb->data)->h_proto != htons(0x0800)) {
		return 0;
	}

	iph = (void *)skb->data+14;
	if (iph) {
		udph = (void *)iph + iph->ihl*4;
		if (iph->protocol==IPPROTO_UDP && (udph->dest == htons(53))) {
			return 1;
		}
	}
	return 0;
}
#elif defined(__CONFIG_APCLIENT_DHCPC__)
int is_dns_packet(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct udphdr *udph;

	if (((struct ethhdr *)skb->data)->h_proto != htons(0x0800)) {
		return 0;
	}

	iph = (void *)skb->data+14;
	if (iph) {
		udph = (void *)iph + iph->ihl*4;
		if (iph->protocol==IPPROTO_UDP && (udph->dest == htons(53))) {
			return 1;
		}
	}
	return 0;
}
#endif /*#if defined(CONFIG_RTL_DNS_TRAP) */



static int count=0;
/**/
/* 
 *  Callback routine to receive packet. Called from device driver
 */
//__MIPS16
void	wrapper_up(struct sk_buff *skb)
{
	struct eth_drv_sc *sc;

	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);
	
	if (skb->dev == NULL) {
		DBG_ERR("%s: skb->dev = NULL!\n", __FUNCTION__);
		return;
	}
#ifdef CONFIG_RTL_819X	
	if(count++ % 10000 == 0) {
#ifdef CYGPKG_IO_WATCHDOG
       	REG32(BSP_WDTCNR) |= (1 << 23); //WDTCLR
#endif

#ifdef CONFIG_RTL_WATCHDOG_SOFT
		externC void softdog_clear_time_c(void);
		softdog_clear_time_c();
#endif
	}
#endif	
#if 1//for fpdebug
	extern unsigned int statistic_total;
	extern unsigned int statistic_skb_fp;

	statistic_total++;
#endif
#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB	
	if(FastPath_Enable()
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
		&&(rtl_CheckPassthruFrame(skb->data)==FAILED)
		#endif
		#if defined(CONFIG_RTL_DNS_TRAP) || defined(__CONFIG_APCLIENT_DHCPC__)
		&& (is_dns_packet(skb)==0)
		#endif
        #ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
        &&(1!=get_multirepeaterFlag(atoi(&skb->dev->name[4])))
        #endif
		)
	{
		
		/*fast bridge*/
		unsigned char *da,*sa;
		struct net_device *dev;		
		struct ether_header * eh;
		da=skb->data;
		sa=skb->data+6;

		#if 1
		/*Only learning in bridge*/
		if(skb->dev && skb->dev->info && (((Rltk819x_t *)(skb->dev->info))->sc) &&
			(((Rltk819x_t *)(skb->dev->info))->sc->sc_arpcom.ac_if.if_bridge))
		{
			addfdbentry(sa,skb->dev);
		}
		
		if(memcmp(da,skb->dev->dev_addr,6) != 0)
		{
			dev=getDevByMac(da);					
         
			if (dev&& netif_running(dev) &&(dev->info)&&(((Rltk819x_t *)(dev->info))->sc)&&
				(((Rltk819x_t *)(dev->info))->sc->sc_arpcom.ac_if.if_bridge)&&
				(strncmp(dev->name, skb->dev->name, 16))){
				dev->hard_start_xmit(skb,dev);
				return;
			}
		}
		
		//dev=getDevByMac(sa);
		//if(1) {
		//	addfdbentry(sa,skb->dev);
		//}
		#endif
		/*fast path. should baseon fastbridge*/
	      eh =(struct ether_header *)skb->data;
		if(1==FastPathSKB_Enter(dev,skb,eh)) {
			#if 1//for fpdebug
			statistic_skb_fp++;
			#endif
			return;
		}
	}
#endif
	DBG_ASSERT(skb->dev->info, "Invalid skb->dev!");

	//if (((Rltk819x_t *)skb->dev->info)->skb == NULL) {
//		diag_printf("(skb->dev->info)->skb=%p  skb=%p\n", ((Rltk819x_t *)skb->dev->info)->skb, skb);
	//}

	if(skb->dev->info == NULL)
	{
		diag_printf("Error!!!%s:skb->dev %p name %s\n",__FUNCTION__,skb->dev,skb->dev->name);
		return;
	}
	((Rltk819x_t *)skb->dev->info)->skb = (void *)skb;	
	sc = ((Rltk819x_t *)skb->dev->info)->sc;

	DBG_ASSERT(sc, "Invalid skb device!");

       /* Tell eCos about the packet */
       (sc->funs->eth_drv->recv)(sc, skb->len);
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	//The cluster should already free by network stack
	//To avoid kfree_skb() free it once again, we set skb->head to NULL.
	skb->head = NULL;
#endif
	kfree_skb(skb);

	((Rltk819x_t *)skb->dev->info)->skb = NULL;		
}

#ifdef ISR_DIRECT
void	wrapper_que_up(struct sk_buff *skb, struct net_device *root_dev)
{
	DBG_ASSERT(skb, "Invalid skb!");
	DBG_ASSERT(skb->dev && skb->dev->info, "Invalid skb->dev!");
	
	skb_queue_tail(&((Rltk819x_t *)root_dev->info)->rx_queue, skb);
}

void wrapper_que_retrieve(struct net_device *dev)
{
	Rltk819x_t *info = (Rltk819x_t *)dev->info;
	struct sk_buff *pskb;
	
	while (skb_queue_len(&info->rx_queue)) {
		pskb = skb_dequeue(&info->rx_queue);
		if (pskb)
			wrapper_up(pskb);
	}
}

int wrapper_que_len(struct net_device *dev)
{
	Rltk819x_t *info = (Rltk819x_t *)dev->info;
	
	return skb_queue_len(&info->rx_queue);
}
#endif /* ISR_DIRECT */

#ifdef TX_PKT_FREE_QUEUE
void wrapper_free_tx_queue(struct net_device *dev)
{
	Rltk819x_t *info = (Rltk819x_t *)dev->info;

	DBG_TRACE("%s, dev=%s\n", __FUNCTION__, dev->name);
	
	while (skb_queue_len(&info->tx_queue)) {
		struct sk_buff *pskb;
		pskb = skb_dequeue(&info->tx_queue);
		if (pskb) {
			struct eth_drv_sc *sc = ((Rltk819x_t *)pskb->dev->info)->sc;			
			(sc->funs->eth_drv->tx_done)(sc, pskb->key, 0);
			kfree_skb(pskb);
		}
	}	
}
#endif


/* 
 * Called from eCOS driver to receive a packet
 */
 __attribute__ ((section(".iram-gen"))) 
//__MIPS16
void wrapper_deliver(Rltk819x_t *info, struct eth_drv_sg *sg_list, int sg_len)
{
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	struct eth_drv_sg *last_sg;
#endif
	struct sk_buff *skb;
	struct mbuf *m;

	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);

	skb = (struct sk_buff *)info->skb;

	//diag_printf("Rx:skb->head=%p skb->data=%p skb->len=%d\n", skb->head, skb->data, skb->len);

	DBG_ASSERT(skb, "No pending rx skb");
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
	m = (struct mbuf *)sg_list[0].buf;
	if (m) {
#if 0
		//if the packet is small, copy it to mbuf and free cluster
		if (skb->len < MHLEN)
		{
			memcpy(m->m_pktdat, skb->data, skb->len);
			sg_list[0].buf = (CYG_ADDRESS)m->m_pktdat;
			sg_list[0].len = sizeof(struct ether_header);
			sg_list[1].buf = (caddr_t)m->m_pktdat+sizeof(struct ether_header);
			sg_list[1].len = skb->len-sizeof(struct ether_header);
			m->m_data = (caddr_t)sg_list[1].buf;
			m->m_len = sg_list[1].len;
			MCLFREE(skb->head);
			skb->head = NULL;
		}
		else
#endif
		{
			sg_list[0].buf = (CYG_ADDRESS)skb->data;
			sg_list[0].len = sizeof(struct ether_header);
			skb_pull(skb, sg_list[0].len);
			sg_list[1].buf = (CYG_ADDRESS)skb->data;
			sg_list[1].len = skb->len;
		
		  	m->m_ext.ext_buf = (caddr_t)skb->head;
			m->m_data = (caddr_t)sg_list[1].buf;
			m->m_len = sg_list[1].len;
			m->m_flags |= M_EXT;
			m->m_ext.ext_size = MCLBYTES;
			m->m_ext.ext_free = NULL;
			m->m_ext.ext_ref = NULL;
#ifdef CYGPKG_NET_OPENBSD_STACK
			m->m_ext.ext_handle = NULL;
#endif
		}

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
		m->m_pkthdr.magic = IS_FORWARD_PKT_MAGIC_NUM;
		m->m_pkthdr.tag.v = skb->tag.v;
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT        
        m->m_pkthdr.forward_rule = skb->forward_rule;
        m->m_pkthdr.index = skb->index;
        m->m_pkthdr.flag_src = skb->flag_src;
        m->m_pkthdr.taged = skb->taged;
#endif
#endif
#if defined  (CONFIG_RTL_HARDWARE_MULTICAST)
	m->m_pkthdr.srcPort = skb->srcPort;
	m->m_pkthdr.srcVlanId = skb->srcVlanId;
#endif

#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_set_wanorlan(&m->m_pkthdr.wanorlan,skb->wanorlan);
#endif

#ifdef CONFIG_RTL_DELAY_REFILL
	m->m_pkthdr.priv = skb->priv;
#endif

#ifdef CONFIG_RTL_MULTI_CLONE_SUPPORT
    memcpy(m->m_pkthdr.cb,skb->cb,sizeof(skb->cb));
#endif

#if defined(CONFIG_RTL_819X)

		m->m_pkthdr.__unused = skb->__unused;

#endif
		/*
		diag_printf("------------start------------------\n");
		diag_printf("sg_list[0].buf=%p\n", sg_list[0].buf);
	        diag_printf("sg_list[0].len=%d\n", sg_list[0].len);
	        diag_printf("sg_list[1].buf=%p\n", sg_list[1].buf);
	        diag_printf("sg_list[1].len=%d\n", sg_list[1].len);
	        diag_printf("m=%p\n", m);
	        diag_printf("m->m_ext.ext_buf=%p\n", m->m_ext.ext_buf);
	        diag_printf("m->m_ext.ext_size=%d\n", m->m_ext.ext_size);
	        diag_printf("m->m_data=%p\n", m->m_data);
	        diag_printf("m->m_len=%d\n", m->m_len);
	        diag_printf("------------end---------------------\n");
	        */
/*
	        diag_printf("m->m_data=%p\n", m->m_data);
	        diag_printf("m->m_len=%d + 14\n", m->m_len);
	        	{
				int i;
				unsigned char *data;
				data = (unsigned char *)skb->data -14;
				for (i=0;(i<m->m_len+14);i++){
					if (i%16 ==0)
 					diag_printf("\n%02X ", data [i]);
					else
						diag_printf("%02X ", data [i]);
					
					}
		}
*/
	}
	else {
		//no mbuf available => drop the packet
		diag_printf("!!!free skb twice!!!\n");
		kfree_skb(skb);
	}
#else
	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
		if (sg_list->buf != (CYG_ADDRESS)0) {
			memcpy((void *)(sg_list->buf), skb->data, sg_list->len);
			skb_pull(skb, sg_list->len);
		}
	}
#endif
}

struct net_device *alloc_etherdev(int sizeof_priv)
{
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
	struct net_device *dev = kmalloc(sizeof(struct net_device), GFP_ATOMIC);
#else
	struct net_device *dev = alloc_local(sizeof(struct net_device));
#endif

	if (dev == NULL) {
		DBG_ERR("%s: allocate net_device failed!\n", __FUNCTION__);
		return NULL;
	}

	memset(dev, '\0', sizeof(*dev));	
	
	if (sizeof_priv > 0) {
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
		dev->priv = kmalloc(sizeof_priv, GFP_ATOMIC);
#else
		dev->priv = alloc_local(sizeof_priv);
#endif

		if (dev->priv == NULL) {
			DBG_ERR("%s: allocate sizeof_priv failed!\n", __FUNCTION__);
			return NULL;
		}
		memset(dev->priv, '\0', sizeof_priv);
	}
	return dev;
}

void wrapper_init(void)
{
	static int initialized = 0;
	void suspend_check_init(void);

	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);
	
	if (0 == initialized) {
		init_skb_pool();
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
		init_skb_data_pool();
#endif
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_KMALLOC_USE_NET_MEMPOOL
		init_local_pool();
#endif
#ifdef CONFIG_RTL_819X_SUSPEND_CHECK
		suspend_check_init();
#endif
		initialized = 1;
	}
}

void wrapper_binding(Rltk819x_t *info, void *dev)
{
	char tmp[40];
	struct net_device *net_dev = (struct net_device *)dev;

	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);

	info->dev = dev;
	net_dev->info = (void *)info;

	info->vector = net_dev->irq;
	net_dev->flags = 0;
	if (!memcmp(net_dev->name, "eth%d", 5)) {
		sprintf(tmp, net_dev->name, eth_idx++);
		strcpy(net_dev->name, tmp);
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
	else if (!memcmp(net_dev->name, "wlan%d-wds%d", 12)) {
		sprintf(net_dev->name, "wlan%d-wds%d", ((info->device_num>>16) & 0xffff), (info->device_num & 0xffff));
	}
#endif
	else if (!memcmp(net_dev->name, "wlan%d", 6)) {
		diag_sprintf(tmp, net_dev->name, ((info->device_num>>16) & 0xffff));
		strcpy(net_dev->name, tmp);
	}

	if (!memcmp(net_dev->name, "eth", 3)) {
		memcpy(info->mac, net_dev->dev_addr, 6);
	}
}

//__IMEM_SECTION__
//__MIPS16__
int wrapper_isr(Rltk819x_t *info)
{
	if (!info->dev) {
		DBG_ERR("%s: wrapper_binding() should be called in advanced!\n", __FUNCTION__);
		return 0;
	}
	return ((((struct net_device *)info->dev)->isr)((struct net_device *)info->dev));
}

//__MIPS16__
void wrapper_dsr(Rltk819x_t *info)
{
	if (!info->dev) {
		DBG_ERR("%s: wrapper_binding() should be called in advanced!\n", __FUNCTION__);
		return;
	}
	(((struct net_device *)info->dev)->dsr)((struct net_device *)info->dev);
}

#if 0
__IMEM_SECTION__
__MIPS16__
void wrapper_free_tx(struct net_device *dev, unsigned long key)
{
	CYG_ASSERT(dev && key, "Invalid dev and key in free wrapper_free_tx()!");

	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);
	struct eth_drv_sc *sc = ((Rltk819x_t *)dev->info)->sc;

	(sc->funs->eth_drv->tx_done)(sc, key, 0);
}
#endif	

#ifdef TX_PKT_FREE_QUEUE
void wrapper_que_free_tx_pkt(struct net_device *root_dev, struct sk_buff *skb)
{
	DBG_ASSERT(root_dev && skb, "Invalid dev and skb in free wrapper_que_free_tx_pkt()!");

	DBG_TRACE("%s, dev=%s, skb=%lx\n", __FUNCTION__, root_dev->name, (u32)skb);

	skb_queue_tail(&((Rltk819x_t *)root_dev->info)->tx_queue, skb);

	/*if queue more than 32 packets, trigger delivery to free. when 5g interface on, if there is NO 5g packets on air 
	 * either no 5g clients connected but packets tx from DUT will be freed and queued in tx_free_queue, so skb exhausted*/
	if(skb_queue_len(&((Rltk819x_t *)root_dev->info)->tx_queue)>32) {
    	((Rltk819x_t *)root_dev->info)->sc->state |= ETH_DRV_NEEDS_DELIVERY;
	}	
}
#endif

void wrapper_start(Rltk819x_t *info)
{
	DBG_TRACE("%s, %d\n", __FUNCTION__, __LINE__);

	if (!info->dev) {
		DBG_ERR("%s: wrapper_binding() should be called in advanced!\n", __FUNCTION__);
		return;
	}

#ifdef ISR_DIRECT	
	skb_queue_head_init(&info->rx_queue);
#endif
#ifdef TX_PKT_FREE_QUEUE
	skb_queue_head_init(&info->tx_queue);
#endif
	
	(((struct net_device *)info->dev)->open)((struct net_device *)info->dev);
	((struct net_device *)info->dev)->flags = 1;
}

void wrapper_stop(Rltk819x_t *info)
{
	if (!info->dev) {
		DBG_ERR("%s: wrapper_binding() should be called in advanced!\n", __FUNCTION__);
		return;
	}

#ifdef ISR_DIRECT
	while (skb_queue_len(&info->rx_queue)) {
		struct sk_buff *pskb;
		pskb = skb_dequeue(&info->rx_queue);
		kfree_skb(pskb);
	}	
#endif
	/*should set flags first when stop to avoid skb leak*/
	((struct net_device *)info->dev)->flags = 0;

	(((struct net_device *)info->dev)->stop)((struct net_device *)info->dev);
#ifdef TX_PKT_FREE_QUEUE
	wrapper_free_tx_queue(info->dev);
#endif
}

int wrapper_can_send(Rltk819x_t *info)
{
	if (!info->dev) {
		DBG_ERR("%s: wrapper_binding() should be called in advanced!\n", __FUNCTION__);
		return 0;
	}
	return (((struct net_device *)info->dev)->can_xmit)((struct net_device *)info->dev);
}

//__MIPS16__
void wrapper_tx(Rltk819x_t *info, struct eth_drv_sg *sg_list, int sg_len,
              int total_len, unsigned long key)
{
	struct net_device *dev = (struct net_device *)info->dev;
	struct eth_drv_sg *last_sg;
	struct sk_buff *skb = NULL;
#ifdef TX_SCATTER
	int i=0;
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	int forward_flag = 1;
#endif
#endif

	/*int j;
	diag_printf("Tx:dev=%s sg_len=%d total_len=%d key=%x\n", dev->name, sg_len, total_len, key);
	for (j=0; j<sg_len; j++) {
		diag_printf("sg_list[%d].buf=%p len=%d\n", j, sg_list[j].buf, sg_list[j].len);
	}*/
	
	DBG_TRACE("%s, dev=%s\n", __FUNCTION__, dev->name);

	DBG_ASSERT(dev, "Invalid dev in free wrapper_tx()!");
	
	if (netif_running(dev)==0) {
		//diag_printf("%s: dev down\n", __FUNCTION__);
		goto free_ret;
	}

#if defined(TX_SCATTER)&&defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	if((rtl_vlan_support_enable)&&(((struct mbuf *)key)->m_pkthdr.magic != IS_FORWARD_PKT_MAGIC_NUM)){
		forward_flag = 0;
	}
	//diag_printf("forward_flag is %d\n", forward_flag);
#endif

#ifdef TX_SCATTER
	//DBG_ASSERT(sg_len <= MAX_LIST_NUM, "Too many sg num!\n");
	if (sg_len <= MAX_LIST_NUM
		#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
		&&(forward_flag==1)
		#endif
	   ) 
	{
		skb = alloc_skb_buf((unsigned char *)sg_list->buf, sg_list->len);
	}
	else 
	{
		//diag_printf("Too many sg num %d!\n", sg_len);
		skb = dev_alloc_skb(total_len);
		if (skb)
			skb_reserve(skb, 2); //make ip header to be 4-byte alignment
	}
#else
	skb = dev_alloc_skb(total_len);
#endif

	if (skb == NULL) {
		//diag_printf("%s: dev_alloc_skb() failed!\n", __FUNCTION__);
		goto free_ret;
	}

	for (last_sg = &sg_list[sg_len]; sg_list < last_sg; ++sg_list) {
#ifdef TX_SCATTER
		if (sg_len <= MAX_LIST_NUM
			#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
			&&(forward_flag==1)
			#endif
		   )
		{
			skb->list_buf[i].buf = (u8 *)sg_list->buf;
			skb->list_buf[i++].len= sg_list->len;
		}
		else 
		{
			memcpy(skb->tail, (void *)(sg_list->buf), sg_list->len);
			skb_put(skb,  sg_list->len);
		}
#else
		memcpy(skb->tail, (void *)(sg_list->buf), sg_list->len);
		skb_put(skb,  sg_list->len);		
#endif	
	}

	DBG_ASSERT(dev->hard_start_xmit, "dev->hard_start_xmit is NULL");

	skb->dev = dev;

#ifdef TX_SCATTER
	if (sg_len <= MAX_LIST_NUM
		#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
		&&(forward_flag==1)
		#endif
	    )
	{
		skb->key = key;
		skb->total_len= total_len;
		skb->list_num = sg_len;
	}
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	skb->tag.v = ((struct mbuf *)key)->m_pkthdr.tag.v;
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT        
    skb->forward_rule = ((struct mbuf *)key)->m_pkthdr.forward_rule;
    skb->index = ((struct mbuf *)key)->m_pkthdr.index;
    skb->flag_src = ((struct mbuf *)key)->m_pkthdr.flag_src;
    skb->taged = ((struct mbuf *)key)->m_pkthdr.taged;
#endif
#endif

#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_set_wanorlan(&skb->wanorlan,((struct mbuf *)key)->m_pkthdr.wanorlan);
#endif

#ifdef CONFIG_RTL_DELAY_REFILL
	skb->priv= ((struct mbuf *)key)->m_pkthdr.priv;
#endif

#ifdef CONFIG_RTL_MULTI_CLONE_SUPPORT
    memcpy(skb->cb,((struct mbuf *)key)->m_pkthdr.cb,sizeof(((struct mbuf *)key)->m_pkthdr.cb));
#endif

	(dev->hard_start_xmit)(skb, dev);
	
#ifdef TX_SCATTER
	if (sg_len <= MAX_LIST_NUM
		#if defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
		&&(forward_flag==1)
		#endif
	    )
		return;
#endif
	
free_ret:	
	(info->sc->funs->eth_drv->tx_done)(info->sc, key, 0);	
}

static struct net_device *wrapper_find_netdev(char *name)
{
    cyg_netdevtab_entry_t *t;
    struct eth_drv_sc *sc;
    Rltk819x_t *info;
    
    for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
		if(!t)continue;
        sc = (struct eth_drv_sc *)t->device_instance;
		if(!sc)continue;
        info = (Rltk819x_t *)sc->driver_private;
		if(!info || !info->dev /*||!((struct net_device *)info->dev)->name*/)
			continue;
        if (strcmp(((struct net_device *)info->dev)->name, name) == 0) {
            return (struct net_device *)info->dev;
        }
    }
    return (struct net_device *)NULL;
}

struct net_device_stats *wrapper_get_stats(char *ifname)
{
	struct net_device *dev;
	
	dev = wrapper_find_netdev(ifname);
	if (dev)
		return dev->get_stats(dev);
	return NULL;
}

void shutdown_netdev(void) 
{
	cyg_netdevtab_entry_t *t;
	struct eth_drv_sc *sc;
	Rltk819x_t *info;
	struct net_device *dev;
	extern void force_stop_wlan_hw(void);

	//diag_printf("Shutdown network interfaces.\n");
	for (t = &__NETDEVTAB__[0]; t != &__NETDEVTAB_END__; t++) {
		sc = (struct eth_drv_sc *)t->device_instance;
		info = (Rltk819x_t *)sc->driver_private;
		dev = (struct net_device *)info->dev;
		if (dev && netif_running(dev) && dev->stop) {
			dev->stop(dev);
		}
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
	force_stop_wlan_hw();
#endif
#ifdef CONFIG_RTL_8367R_SUPPORT
	// for boot code tftp feature after did the kernel reboot, of course rtk_vlan_init() can be moved to boot code, but the boot code size is near 24Kbytes
	{	
//	extern int rtk_vlan_init(void);
//	rtk_vlan_init();
		extern void rtl8367rb_reset(void);
		rtl8367rb_reset();
	}
#endif

}

//----------------------------------------------------------------------------
int rtk_atoi(char *s)
{
	int k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}
//----------------------------------------------------------------------------

#ifdef CONFIG_RTL_819X_SUSPEND_CHECK
//#define INTR_HIGH_WATER_MARK 1850 //for window size = 1, based on LAN->WAN test result
//#define INTR_LOW_WATER_MARK  1150
//#define INTR_HIGH_WATER_MARK 9190 //for window size = 5, based on LAN->WAN test result
//#define INTR_LOW_WATER_MARK  5500
#define INTR_HIGH_WATER_MARK 3200  //for window size = 5, based on WLAN->WAN test result
#define INTR_LOW_WATER_MARK  2200
#define INTR_WINDOW_SIZE_MAX 10
static int suspend_check_enable = 1;
static int suspend_check_high_water_mark = INTR_HIGH_WATER_MARK;
static int suspend_check_low_water_mark = INTR_LOW_WATER_MARK;
static int suspend_check_win_size = 5;
static struct timer_list suspend_check_timer;
static int suspend_check_index=0;
static int eth_int_count[INTR_WINDOW_SIZE_MAX];
static int wlan_int_count[INTR_WINDOW_SIZE_MAX];
int cpu_can_suspend = 1;

void suspend_check_cmd_dispatch(int argc, char *argv[])
{
	if (strcmp(argv[0], "on")==0) {
		suspend_check_enable = 1;
		mod_timer(&suspend_check_timer, jiffies + 100);
	}
	else if (strcmp(argv[0], "off")==0) {
		suspend_check_enable = 0;
		if (timer_pending(&suspend_check_timer))
			del_timer_sync(&suspend_check_timer);
	}
	else if (strcmp(argv[0], "winsize")==0) {
		//diag_printf("argc=%d\n", argc);
		if (argc > 1) {
			suspend_check_win_size = rtk_atoi(argv[1]);
			if (suspend_check_win_size >= INTR_WINDOW_SIZE_MAX)
				suspend_check_win_size = INTR_WINDOW_SIZE_MAX - 1;
		}
	}
	else if (strcmp(argv[0], "high")==0) {
		if (argc > 1) {
			suspend_check_high_water_mark = rtk_atoi(argv[1]);
		}
	}
	else if (strcmp(argv[0], "low")==0) {
		if (argc > 1) {
			suspend_check_low_water_mark = rtk_atoi(argv[1]);
		}
	}
	else if (strcmp(argv[0], "info")==0) {
		diag_printf("enable=%d, winsize=%d(max=%d), high=%d, low=%d, cpu_can_suspend=%d\n",
			suspend_check_enable, suspend_check_win_size,
			INTR_WINDOW_SIZE_MAX, suspend_check_high_water_mark,
			suspend_check_low_water_mark, cpu_can_suspend);
	}
}

static void suspend_check_timer_fn(void *arg)
{
	int count, j;

	suspend_check_index++;
	if (INTR_WINDOW_SIZE_MAX <= suspend_check_index)
		suspend_check_index = 0;
	eth_int_count[suspend_check_index] = rtk_interrupt_count[BSP_SW_IRQ];
	wlan_int_count[suspend_check_index] = rtk_interrupt_count[BSP_PCIE_IRQ];
	j = suspend_check_index - suspend_check_win_size;
	if (j < 0)
		j += INTR_WINDOW_SIZE_MAX;
	count = (eth_int_count[suspend_check_index] - eth_int_count[j]) + 
		(wlan_int_count[suspend_check_index]- wlan_int_count[j]); //unit: number of interrupt occurred

	if (cpu_can_suspend) {
		if (count > suspend_check_high_water_mark) {
			cpu_can_suspend = 0;
			//diag_printf("\n<<<RTL8196C LEAVE SLEEP>>>\n"); /* for Debug Only*/
		}
	}
	else {
		if (count < suspend_check_low_water_mark) {
			cpu_can_suspend = 1;
			//diag_printf("\n<<<RTL8196C ENTER SLEEP>>>\n"); /* for Debug Only*/
		}
	}
#if 0 /* for Debug Only*/
	diag_printf("###index=%d, count=%d (%d+%d) cpu_can_suspend=%d###\n",
		suspend_check_index, count, 
		(eth_int_count[suspend_check_index] - eth_int_count[j]), 
		(wlan_int_count[suspend_check_index]- wlan_int_count[j]),
		cpu_can_suspend);
#endif
	mod_timer(&suspend_check_timer, jiffies + 100);
}

void suspend_check_init(void)
{
	int i;

	for (i=0; i<INTR_WINDOW_SIZE_MAX; i++) {
		wlan_int_count[i] = 0;
		eth_int_count[i] = 0;
	}
	suspend_check_timer.data = (unsigned long)0;
	suspend_check_timer.function = suspend_check_timer_fn;
	init_timer(&suspend_check_timer);
	mod_timer(&suspend_check_timer, jiffies + 100);
}
#endif

#if defined(CONFIG_RTL_819X)
extern int rtl_isWanDevDecideByName(unsigned char *name);
int rtl_isWanDevDecideByIfp(struct ifnet *ifp)
{
	int ret = 0;
	struct eth_drv_sc	*sc;
	Rltk819x_t 	*info;
	struct net_device	*dev;

	sc = (struct eth_drv_sc *) ifp->if_softc;
	if (sc==NULL) {
		if (0 == strcmp(ifp->if_name, "ppp"))
			return 1;
		else
			return 0;
	}
	info = (Rltk819x_t *)sc->driver_private;
	dev = (struct net_device *)info->dev;
	#if defined(CONFIG_RTL_819X_SWCORE)
	ret = rtl_isWanDevDecideByName(dev->name);
	#endif
	
	return ret;
}

struct net_device * rtl_getDevByIfp(struct ifnet *ifp)
{
	struct eth_drv_sc	*sc;
	Rltk819x_t 	*info;
	struct net_device	*dev;
	
	sc = (struct eth_drv_sc *) ifp->if_softc;
	info = (Rltk819x_t *)sc->driver_private;
	dev = (struct net_device *)info->dev;
	return dev;
}

struct net_device * rtl_getDevByName(char *name)
{
	struct ifnet *ifp;
	ifp=ifunit(name);
	if(ifp)
		return rtl_getDevByIfp(ifp);
	return NULL;
}

#endif
