
#include <sys/param.h>
#if !defined(CONFIG_RTK_VLAN_SUPPORT)
//#include <netinet/fastpath/rtl_types.h>
#include <rtl/rtl_types.h>
#endif

//#include <net/netfilter/nf_conntrack.h>
//#include <net/netfilter/nf_conntrack_core.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <net/if.h>


#ifdef CONFIG_BRIDGE
#include <bridge/br_private.h>
#endif

/*
#if defined (FAST_PPTP) || defined(FAST_L2TP)
	#include <net/ip.h>
#endif
*/

#if defined(CONFIG_NET_SCHED)
#include <linux/netfilter_ipv4/ip_tables.h>
extern int gQosEnabled;
#endif
//#include <net/rtl/features/rtl_features.h>
//#include <netinet/fastpath/rtl_queue.h>
#include <rtl/rtl_queue.h>
#include <netinet/fastpath/fastpath_core.h>
#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
#include <net/rtl/features/rtl_features.h>
#endif
//#include <net/rtl/rtl865x_nat.h>
//#include <net/rtl/features/rtl_ps_log.h>
#include  <net/if_types.h>

#if defined (DUMMYNET)
#define CONFIG_RTL_QOS 1
#endif

#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
static int	rtl_fp_gc_rx_threshold;
#endif
__DRAM_GEN static int fast_nat_fw = 1;

unsigned int	_br0_ip = 0;
unsigned int	_br0_mask = 0;

int ppp_comp_flag =0;

int get_fast_pppoe_flag()
{	
#ifdef CONFIG_RTL_FAST_PPPOE
	extern int fast_pppoe_fw; 
	return fast_pppoe_fw;
#else
	return 0;
#endif
}
int get_fast_l2tp_flag()
{
#ifdef FAST_L2TP
	extern int fast_l2tp_fw;
	return fast_l2tp_fw;
#else
	return 0;
#endif
}
int get_fast_pptp_flag()
{
#ifdef FAST_PPTP
	extern int fast_pptp_fw;
	return fast_pptp_fw;
#else
	return 0;
#endif
}

#ifdef CONFIG_RTL_FAST_PPPOE
extern struct pppoe_info fast_pppoe_info[];

static struct pppoe_info* fast_find_pppoe_info( unsigned short sid, unsigned char *peer_mac)
{
    int i;


    for(i=0; i<MAX_PPPOE_ENTRY; i++)
    {
        if(fast_pppoe_info[i].valid==0)
        {
            continue;
        }

        if((fast_pppoe_info[i].sid==sid) && (memcmp(fast_pppoe_info[i].peer_mac,peer_mac,6)==0))
        {
            return &fast_pppoe_info[i];
        }

    }
    return NULL;
}
#endif

#ifdef CONFIG_RTL_PPPOE_DIRECT_REPLY
int magicNum=-1;
void clear_magicNum(struct sk_buff *pskb)
{
	//diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	magicNum = -1;
}

int is_pppoe_lcp_echo_req(struct sk_buff *skb)
{
	unsigned char *mac_hdr=NULL;
	unsigned short sid;
	struct pppoe_info* pppoe_info_ptr=NULL;

	if(skb->dev==NULL)
	{
		return 0;
	}
	
	if(strncmp(skb->dev->name, "ppp" ,3)==0)
	{
		return 0;
	}
#ifdef __ECOS
	mac_hdr=skb->data;
#else
	mac_hdr=rtl_skb_mac_header(skb);
#endif
	if(mac_hdr==NULL)
	{
		return 0;
	}

	sid=ntohs(*(unsigned short *)(&mac_hdr[16]));
	pppoe_info_ptr=fast_find_pppoe_info(sid,&mac_hdr[6]);

	if((pppoe_info_ptr==NULL))
	{
		//diag_printf("\n%s:%d sid=%d dst_mac=%02x:%02x:%02x:%02x:%02x:%02x\n",__FUNCTION__,__LINE__,sid,mac_hdr[0],mac_hdr[1],mac_hdr[2],mac_hdr[3],mac_hdr[4],mac_hdr[5]);
		return 0;
	}	
	
	if( (mac_hdr[12]!=0x88) || (mac_hdr[13]!=0x64) \
		||(mac_hdr[20]!=0xc0) || (mac_hdr[21]!=0x21) \
		|| (mac_hdr[22]!=0x09))
	{		
		return 0;
	}
	return 1;

}
void extract_magicNum(struct sk_buff *skb)
{
	unsigned char *mac_hdr = skb->data;
	int i;
	int payloadLen=0;
	unsigned char type;
	unsigned char len;
	unsigned char *ptr;
	
	/*
	if( (*(unsigned short *)(&mac_hdr[12])==0x8864) \
		&&(*(unsigned short *)(&mac_hdr[20])==0xc021))
	{
		panic_printk("skb->dev is %s\n",skb->dev->name);
		for(i=0;i<32;i++)
		{
				panic_printk("0x%x\t",mac_hdr[i]);
				if(i%8==7)
				{
					panic_printk("\n");
				}
		}

	}	
	*/
	if(skb->dev==NULL)
	{
		return 0;
	}
	
	if(strncmp(skb->dev->name, "ppp" ,3)==0)
	{
		return;
	}
	
	if((mac_hdr[12]!=0x88) || (mac_hdr[13]!=0x64))
	{
		return;
	}
	
	
	if((mac_hdr[20]!=0xc0) || (mac_hdr[21]!=0x21))
	{
		return;
	}

	/*lcp configuration request*/
	if(mac_hdr[22]==0x01)
	{
	
		payloadLen=(mac_hdr[24]<<8)+mac_hdr[25];	
		payloadLen=payloadLen-4;
		ptr=(mac_hdr+26);

		while(payloadLen>0)
		{
			/*parse tlv option*/
			type=*ptr;
			len=*(ptr+1);
			
			//diag_printf("%s:%d,type is %d\n",__FUNCTION__,__LINE__,type);
			if(type==0x05) /*magic number option*/
			{
				memcpy(&magicNum, ptr+2 , 4);
				//diag_printf("%s:%d,magicNum is 0x%x\n",__FUNCTION__,__LINE__,magicNum);
				break;
			}

			if(len>payloadLen)
			{
				break;
			}
			ptr=ptr+len;
			payloadLen=payloadLen-len;
			
		}
	}
	else if(mac_hdr[22]==0x09) /*lcp echo request*/
	{
		ptr=(mac_hdr+26);
		memcpy(&magicNum, ptr , 4);
		//diag_printf("%s:%d,magic number is 0x%x\n",__FUNCTION__,__LINE__,magicNum);
	}
	return;
}

int direct_send_reply(struct sk_buff *skb)
{	
	unsigned char tmp[6];
		if(strncmp(skb->dev->name, "ppp" ,3)!=0)
	{
		//skb_push(skb,14);
		// swap src and dst mac  	
		memcpy(tmp,skb->data,6);
		memcpy(skb->data,(skb->data+6),6);
		memcpy((skb->data+6),tmp,6);		
			
		// build ppp session header
		skb->data[22] = 0x0a;			//reply num
		memcpy((skb->data+26),&magicNum ,4);
		skb->dev->hard_start_xmit(skb,skb->dev);
		//diag_printf("%s:%d, direct_send_reply  tx\n",__FUNCTION__,__LINE__);		
	
		return 1;
	}
	return 0;
}
#endif

#if 1

static int fast_skb_fw = 1;

void set_fast_skb_fw(int value)
{
	fast_skb_fw = value;
}

int get_fast_skb_fw()
{
	return fast_skb_fw;
}

int get_fast_fw()
{
	return fast_nat_fw;
}

#if 0
unsigned char * rtl_get_skb_data(struct sk_buff *pdata)
{
	return pdata->data;
}
#endif
#endif

#ifdef __ECOS
#define IS_CLASSD_ADDR(__ipv4addr__)				((((uint32)(__ipv4addr__)) & 0xf0000000) == 0xe0000000)
long fp_jiffies;
#endif

#if defined (DUMMYNET)&&defined (CONFIG_RTL_819X)
extern int set_QosEnabled( int enabled);
extern  int get_QosEnabled( void);
#endif

#if defined(IMPROVE_QOS) || defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/arp.h>
#include <net/rtl/rtl865x_netif.h>
//To query hardware address based on IP through arp table of dev
int arp_req_get_ha(__be32 queryIP, struct net_device *dev, unsigned char * resHwAddr)
{
	__be32 ip = queryIP;
	struct neighbour *neigh;
	int err = -ENXIO;

	neigh = neigh_lookup(&arp_tbl, &ip, dev);
	if (neigh) {
		read_lock_bh(&neigh->lock);
		memcpy(resHwAddr, neigh->ha, dev->addr_len);
		read_unlock_bh(&neigh->lock);
		neigh_release(neigh);
		err = 0;
	}
	//else
	//{
	//	resHwAddr=NULL;
	//}

	return err;
}
//EXPORT_SYMBOL(arp_req_get_ha);
#endif


#if defined(FAST_PATH_SPI_ENABLED)
int fast_spi =1;
static struct proc_dir_entry *res_spi=NULL;
static int spi_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "fast_spi %s\n", fast_spi==1?"Enabled":"Disabled");

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;

}
static int spi_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmpbuf[16];

	memset(tmpbuf, 0, sizeof(tmpbuf));
	if (buffer && !copy_from_user(tmpbuf, buffer, count))
		sscanf(tmpbuf, "%d", &fast_spi);

	return count;
}

#endif


#if defined(CONFIG_PROC_FS)
static struct proc_dir_entry *res1=NULL;
static int read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len = 0;

	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	len = sprintf(page, "fastpath %s, GC_RX_Count %d, Status: %d\n", fast_nat_fw!=0?"Enabled":"Disabled", rtl_fp_gc_rx_threshold, rtl_newGC_session_status_flags);
	#else
	len = sprintf(page, "fastpath: [%d]\n", fast_nat_fw+10);
	#endif
	len = fastpath_dump_napt_entry_num(page, len);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;

}
static int write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmpbuf[16];
	struct net *net;

	if (count < 2)
		return -EFAULT;

	memset(tmpbuf, 0, sizeof(tmpbuf));
	if (buffer && !copy_from_user(tmpbuf, buffer, count))  {
		if (tmpbuf[0] == '2'&&count==2){
			/* first byte == 2, second byte == "enter" */
			for_each_net(net) {
				nf_conntrack_flush(net, 0, 0);		//clean conntrack table
			}
			#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
			#if defined(CONFIG_RTL_8198) || defined (CONFIG_RTL_8196CT)
			rtl865x_nat_reinit();
			/* the following 2 values MUST set behind reinit nat module	*/
			//rtl_nat_expire_interval_update(RTL865X_PROTOCOL_TCP, tcp_get_timeouts_by_state(TCP_CONNTRACK_ESTABLISHED));
			//rtl_nat_expire_interval_update(RTL865X_PROTOCOL_UDP, nf_ct_udp_timeout>nf_ct_udp_timeout_stream?nf_ct_udp_timeout:nf_ct_udp_timeout_stream);
			#endif
			#endif
		}else{
			sscanf(tmpbuf, "%d", &fast_nat_fw);
			#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
			if (fast_nat_fw>2) {
				rtl_fp_gc_rx_threshold = fast_nat_fw;
			}
			#endif
		}
		return count;
	}

	return -EFAULT;
}
#endif

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
int32 rtl_qosGetSkbMarkByNaptEntry(rtl865x_napt_entry *naptEntry, rtl865x_qos_mark *qosMark, struct sk_buff *pskb)
{
	struct iphdr *iph;
	struct tcphdr *tcphupuh;  //just keep one , don't care tcp or udp //
	u_int ori_saddr, ori_daddr;
	u_short ori_sport, ori_dport;
	struct net_device	*lanDev, *wanDev;
	struct dst_entry *dst_tmp;
	unsigned char oriSrcMac[6],oriDstMac[6],resMac[14];
	u_short proto;
	unsigned char pppProto[2],ipProto[2];
	__u32 oriSkbMark;
	unsigned long irq_flags;
	uint32 preMark, postMark;

	if(pskb==NULL)
		return FAILED;

	//initial
	pppProto[0]=0x00;
	pppProto[1]=0x21;
	ipProto[0]=0x08;
	ipProto[1]=0x00;

	lanDev=rtl865x_getLanDev();
	wanDev=rtl865x_getWanDev();
	proto = ntohs(pskb->protocol);
	iph = ip_hdr(pskb);
	tcphupuh = (struct tcphdr*)((__u32 *)iph + iph->ihl);

	//To bak origal protol mac
	memcpy(oriSrcMac,eth_hdr(pskb)->h_source,ETH_ALEN);
	memcpy(oriDstMac,eth_hdr(pskb)->h_dest,ETH_ALEN);

	//Bak orignal skb mark
	oriSkbMark=pskb->mark;

	//check ip-based qos rule at iptables mangle table
	//To record original info
	ori_saddr=iph->saddr;
	ori_sport=tcphupuh->source;
	ori_daddr=iph->daddr;
	ori_dport=tcphupuh->dest;

	/* for dst mac match, please refer to the xt_mac.c */
	dst_tmp = pskb->dst;
	pskb->dst = NULL;

	//For uplink
#if defined(CONFIG_NET_SCHED)
	preMark = 0;
	postMark= 0;
	{
	//Replace source addr to check uplink mark
	iph->saddr=naptEntry->intIp;
	tcphupuh->source=naptEntry->intPort;
	iph->daddr=naptEntry->remIp;
	tcphupuh->dest=naptEntry->remPort;

	memset(resMac,0,14);
	if((lanDev!=NULL)&&(arp_req_get_ha(naptEntry->intIp,lanDev,resMac)==0))
	{
		//Patch for pppoe wantype: run udp chariot
		//bak skb mac header
		if((memcmp(pskb->data-2, pppProto,2)==0)	//equal 0x0021
			&&(skb_mac_header_was_set(pskb)==1)
			&&(eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
		{
				skb_set_mac_header(pskb, -22);
		}

		//Replace source mac addr to check uplink mark
		memcpy(eth_hdr(pskb)->h_source,resMac, ETH_ALEN);
		memcpy(eth_hdr(pskb)->h_dest,lanDev->dev_addr, ETH_ALEN);
	}

	pskb->mark=0;//initial
	if(proto == ETH_P_IP){
		(list_empty(&nf_hooks[PF_INET][NF_IP_PRE_ROUTING]))?: \
			ipt_do_table(pskb, NF_IP_PRE_ROUTING, lanDev,wanDev,\
			dev_net(lanDev)->ipv4.iptable_mangle);
	}

	DEBUGP_API("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
					lanDev?lanDev->name:"NULL",
					wanDev?wanDev->name:"NULL",
					pskb->inDev?pskb->inDev->name:"NULL",
					pskb->dev?pskb->dev->name:"NULL", pskb->mark);
	preMark = pskb->mark;

	//Replace dest addr to check uplink mark
	iph->saddr=naptEntry->extIp;
	tcphupuh->source=naptEntry->extPort;
	iph->daddr=naptEntry->remIp;
	tcphupuh->dest=naptEntry->remPort;

	memset(resMac,0,14);
	if((wanDev!=NULL)&&(arp_req_get_ha(naptEntry->remIp,wanDev,resMac)==0))
	{
		//Patch for pppoe wantype: run udp chariot
		//bak skb mac header
		if((memcmp(pskb->data-2, pppProto,2)==0)	//equal 0x0021
			&&(skb_mac_header_was_set(pskb)==1)
			&&(eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
		{
				skb_set_mac_header(pskb, -22);
		}

		//Replace source mac addr to check uplink mark
		memcpy(eth_hdr(pskb)->h_dest,resMac, ETH_ALEN);
		memcpy(eth_hdr(pskb)->h_source,wanDev->dev_addr, ETH_ALEN);
	}

	pskb->mark=0;//initial
	if(proto == ETH_P_IP){
		(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
			ipt_do_table(pskb, NF_IP_POST_ROUTING, lanDev, wanDev,\
			dev_net(wanDev)->ipv4.iptable_mangle);
	}
	DEBUGP_API("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
					lanDev?lanDev->name:"NULL",
					wanDev?wanDev->name:"NULL",
					pskb->inDev?pskb->inDev->name:"NULL",
					pskb->dev?pskb->dev->name:"NULL", pskb->mark);
	postMark= pskb->mark;
	}

	qosMark->uplinkMark=(postMark?postMark:preMark);
#endif

	//for downlink
#if defined(CONFIG_NET_SCHED)
	preMark = 0;
	postMark = 0;
	{
		//Replace source addr to check uplink mark
		iph->saddr=naptEntry->remIp;
		tcphupuh->source=naptEntry->remPort;
		iph->daddr=naptEntry->extIp;
		tcphupuh->dest=naptEntry->extPort;

		memset(resMac,0,14);
		if((wanDev!=NULL)&&(arp_req_get_ha(naptEntry->remIp,wanDev,resMac)==0))
		{
			//Patch for pppoe wantype: run udp chariot
			if((memcmp(pskb->data-2, pppProto,2)==0)	//equal 0x0021
				&&(skb_mac_header_was_set(pskb)==1)
				&&(eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
			{
				skb_set_mac_header(pskb, -22);
			}

			//Replace source mac addr to check uplink mark
			memcpy(eth_hdr(pskb)->h_source,resMac, ETH_ALEN);
			memcpy(eth_hdr(pskb)->h_dest, wanDev->dev_addr, ETH_ALEN);
		}

		pskb->mark=0;//initial
		if(proto == ETH_P_IP){
			(list_empty(&nf_hooks[PF_INET][NF_IP_PRE_ROUTING]))?: \
				ipt_do_table(pskb, NF_IP_PRE_ROUTING, wanDev,lanDev,\
				dev_net(wanDev)->ipv4.iptable_mangle);
		}
		DEBUGP_API("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
						lanDev?lanDev->name:"NULL",
						wanDev?wanDev->name:"NULL",
						pskb->inDev?pskb->inDev->name:"NULL",
						pskb->dev?pskb->dev->name:"NULL", pskb->mark);
		preMark = pskb->mark;

		//Replace dest addr to check uplink mark
		iph->saddr=naptEntry->remIp;
		tcphupuh->source=naptEntry->remPort;
		iph->daddr=naptEntry->intIp;
		tcphupuh->dest=naptEntry->intPort;

		memset(resMac,0,14);
		if ((lanDev!=NULL)&&(arp_req_get_ha(naptEntry->intIp,lanDev,resMac)==0))
		{
			//Patch for pppoe wantype: run udp chariot
			if((memcmp(pskb->data-2, pppProto,2)==0)	//equal 0x0021
				&&(skb_mac_header_was_set(pskb)==1)
				&&(eth_hdr(pskb)->h_proto!=ntohs(0x0800))) //not equal to 0x0800
			{
				skb_set_mac_header(pskb, -22);
			}
			//Replace dest mac addr and  hh data mac to check uplink mark
			memcpy(eth_hdr(pskb)->h_dest,resMac,ETH_ALEN);
			memcpy(eth_hdr(pskb)->h_source, lanDev->dev_addr, ETH_ALEN);
		}
		pskb->mark=0;//initial

		if(proto == ETH_P_IP){
			(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
				ipt_do_table(pskb, NF_IP_POST_ROUTING, wanDev, lanDev,\
				dev_net(lanDev)->ipv4.iptable_mangle);
		}
		DEBUGP_API("[%s][%d]:[%s][%s][%s][%s][%d]\n", __FUNCTION__, __LINE__,
						lanDev?lanDev->name:"NULL",
						wanDev?wanDev->name:"NULL",
						pskb->inDev?pskb->inDev->name:"NULL",
						pskb->dev?pskb->dev->name:"NULL", pskb->mark);
		postMark= pskb->mark;
	}
	qosMark->downlinkMark=(postMark?postMark:preMark);
#endif

	//Back to original value
	//Back to orignal protol mac
	memcpy(eth_hdr(pskb)->h_source,oriSrcMac, ETH_ALEN);
	memcpy(eth_hdr(pskb)->h_dest,oriDstMac, ETH_ALEN);

	//Back to original skb mark
	pskb->mark=oriSkbMark;

	//back to original info
	iph->saddr=ori_saddr;
	tcphupuh->source=ori_sport;
	iph->daddr=ori_daddr;
	tcphupuh->dest=ori_dport;

	pskb->dst = dst_tmp;

	if(lanDev)
		dev_put(lanDev);

	if(wanDev)
		dev_put(wanDev);

	return SUCCESS;
}
#endif

#if defined(IMPROVE_QOS)
void fastpath_set_qos_mark(struct sk_buff *skb, unsigned int preRouteMark, unsigned int postRouteMark)
{
	if(skb->mark == 0)
		skb->mark = (postRouteMark?postRouteMark:preRouteMark);
}
#endif

/*
#if	defined(FAST_L2TP)
static inline void enter_fast_path_fast_l2tp_pre_process(struct sk_buff *skb)
{
	struct net_device *l2tprx_dev;
	struct in_device *skbIn_dev;
	struct net_device *skbNetDevice;

	if(fast_l2tp_fw){
		l2tprx_dev = skb->dev;
		skbIn_dev = (struct in_device *)skb->dev->ip_ptr;
		if(skbIn_dev == NULL){
			if ((skbNetDevice = __dev_get_by_name(&init_net,l2tprx_dev->name)) != NULL){
				if((skbIn_dev= (struct in_device*)skbNetDevice->ip_ptr) != NULL)
					skb->dev->ip_ptr = (void *)skbIn_dev;
			}
		}
	}
}
#endif
*/

#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
static inline int rtl_fp_gc_status_check_priority(uint32 sIp, uint32 dIp, uint16 sPort, uint16 dPort)
{
	#define	RTL_FP_GC_HIGH_PRIORITY_PORT_CHECK(__FP__GC__PORT__)	((sPort!=(__FP__GC__PORT__))&&dPort!=(__FP__GC__PORT__))

	if ( (dIp!=_br0_ip && sIp!=_br0_ip) &&
		(sPort>1024 && dPort>1024) &&
		(sPort!=8080 && dPort!=8080) &&
		(!(IS_CLASSD_ADDR(dIp))) &&
		(!(IS_BROADCAST_ADDR(dIp))) &&
		(!(IS_ALLZERO_ADDR(sIp))))
	{
		return NET_RX_DROP;
	}
	else
		return NET_RX_SUCCESS;
	#undef	RTL_FP_GC_HIGH_PRIORITY_PORT_CHECK
}

static inline int rtl_fp_gc_status_check(struct iphdr *iph, struct tcphdr *tcph, struct sk_buff *skb)
{
	uint32 sIp, dIp;
	uint16 sPort, dPort;
	int	ret;
	static int	rx_count=0;

	ret = NET_RX_SUCCESS;
	if ((rtl_newGC_session_status_flags!=RTL_FP_SESSION_LEVEL_IDLE)&&time_after_eq(rtl_newGC_session_status_time, jiffies)) {
		sIp = iph->saddr;
		dIp = iph->daddr;
		sPort = tcph->source;
		dPort = tcph->dest;
		if (rtl_fp_gc_status_check_priority(sIp, dIp, sPort, dPort)==NET_RX_DROP) {
			kfree_skb(skb);
			ret = NET_RX_DROP;
		}
	} else {
		if (rtl_newGC_session_status_flags==RTL_FP_SESSION_LEVEL3) {
			if ((rx_count++)>rtl_fp_gc_rx_threshold) {
				rx_count = 0;
				rtl_newGC_session_status_time = jiffies+RTL_FP_SESSION_LEVEL3_INTERVAL;
			}
		} else if (rtl_newGC_session_status_flags==RTL_FP_SESSION_LEVEL1) {
			rx_count += RTL_FP_SESSION_LEVEL1_RX_WEIGHT;
			if ((rx_count)>rtl_fp_gc_rx_threshold) {
				rx_count = 0;
				rtl_newGC_session_status_time=jiffies+RTL_FP_SESSION_LEVEL1_INTERVAL;
			}
		}
	}

	return ret;
}
#endif

int fast_path_pre_process_check(struct ip *iph, struct tcphdr *tcphupuh, struct mbuf *m)
{
	/*
	#if	defined(FAST_L2TP)
	enter_fast_path_fast_l2tp_pre_process(m);
	#endif
	*/
	
	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	if ((iph->protocol==IPPROTO_TCP) &&
		(tcphupuh->syn) && (!(tcphupuh->ack))) {
		return rtl_fp_gc_status_check(iph, tcphupuh, m);
	}
	#endif

	return NET_RX_PASSBY;
}



static inline int enter_fast_path_fast_l2tp_post_process(struct sk_buff *skb, struct ifnet  *ifp)
{
#if	defined(FAST_L2TP)
	if (fast_l2tp_fw && skb->dev && (ifp->if_type == IFT_PPP))
	{
	
		#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
		if (skb->len > ifp->if_mtu) 
		{
			if (fast_l2tp_to_wan_check(ifp, skb) == 0)
				return NET_RX_PASSBY;
				
			/*rtl_ip_fragment_for_fastpath always will not return 0 now, skb will be
			xmit or kfree in this func, so the return value is NET_RX_DROP.*/
			if (rtl_ip_fragment_for_fastpath(ifp, skb, NULL, NULL, fast_l2tp_to_wan2))
				return NET_RX_DROP;
		} else
		#endif
		if (fast_l2tp_to_wan((void*)skb,ifp)) // success
		{
			return NET_RX_DROP;
		} else
		{
			/*if fast l2tp can not handle packet which go to wan ppp interface
			   then return NET_RX_PASSBY*/
			return NET_RX_PASSBY;
		}
	}
#endif	
	return NET_RX_SUCCESS;
}




static inline int enter_fast_path_fast_pppoe_post_process(struct sk_buff *skb, struct ifnet  *ifp)
{
	int ret;
	ret=NET_RX_SUCCESS;
#if defined(CONFIG_RTL_FAST_PPPOE)
	#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
	if (skb->len > ifp->if_mtu) 
	{
		ret = fast_pppoe_xmit_check(skb, ifp);
		if (ret != 1)
			return ret;
		/*rtl_ip_fragment_for_fastpath always will not return 0 now, skb will be
		xmit or kfree in this func, so the return value is NET_RX_DROP.*/
		if (rtl_ip_fragment_for_fastpath(ifp, skb, NULL, NULL, fast_pppoe_xmit2))
			return NET_RX_DROP;
	} else
	#endif
	if ((ret=fast_pppoe_xmit(skb,ifp))==NET_RX_DROP) // success
	{
		return NET_RX_DROP;
	}
#endif
	return ret;
}
static inline int enter_fast_path_fast_pptp_post_process_skb(struct sk_buff *skb, struct ifnet  *ifp)
{
	int ret;
	ret=NET_RX_SUCCESS;
#if defined(FAST_PPTP)
	#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
	if (skb->len > ifp->if_mtu) 
	{
		if (fast_pptp_to_wan_check(ifp, skb) == 0)
			return NET_RX_SUCCESS;
		/*Acturally, rtl_ip_fragment_for_fastpath will never return 0.*/
		if (rtl_ip_fragment_for_fastpath(ifp, skb, NULL, NULL, fast_pptp_to_wan_skb2))
			return NET_RX_DROP;
	} else
	#endif
	if ((ret=fast_pptp_to_wan_skb(skb,ifp))==NET_RX_DROP) // success
	{
		return NET_RX_DROP;
	}
#endif
	return ret;
}


static inline int enter_fast_path_fast_pptp_post_process(struct mbuf **pbuf, struct ifnet  *ifp)
{
#if	defined(FAST_PPTP) 
	if (fast_pptp_fw && (ifp->if_type == IFT_PPP))
	{
		if (fast_pptp_to_wan(pbuf,ifp)) // success
		{
			return NET_RX_DROP;
		}
	}
#endif	
	return NET_RX_SUCCESS;
}


#ifdef __ECOS
struct net_device *rtl_get_skb_dev(struct sk_buff* skb)
{
	return skb->dev;
}

#if 0
int rtl_call_skb_ndo_start_xmit_2(struct sk_buff *skb)
{
	return skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
}
#endif

unsigned int rtl_skb_headroom(struct sk_buff *skb)
{
	return skb_headroom(skb);
}
unsigned char *rtl_skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull(skb,len);
}

unsigned char *rtl_skb_push(struct sk_buff *skb, unsigned int len)
{
	return skb_push(skb,len);
}
unsigned int rtl_get_skb_pppoe_flag(struct sk_buff* skb)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	return skb->pppoe_flag;
	#else
	return 0;
	#endif
}

void rtl_set_skb_pppoe_flag(struct sk_buff* skb, unsigned int pppoe_flag)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	skb->pppoe_flag=pppoe_flag;
	#endif
	return;
}

unsigned int rtl_get_skb_l2tp_flag(struct sk_buff* skb)
{
	#if defined (FAST_L2TP)
	return skb->l2tp_flag;
	#else
	return 0;
	#endif
}

void rtl_set_skb_l2tp_flag(struct sk_buff* skb, unsigned int flag)
{
	#if defined (FAST_L2TP)
	skb->l2tp_flag=flag;
	#endif
	return;
}

unsigned int rtl_get_skb_l2tp_offset(struct sk_buff* skb)
{
	#if defined (FAST_L2TP)
	return skb->l2tp_offset;
	#else
	return 0;
	#endif
}

void rtl_set_skb_l2tp_offset(struct sk_buff* skb, unsigned int offset)
{
	#if defined (FAST_L2TP)
	skb->l2tp_offset=offset;
	#endif
	return;
}

unsigned int rtl_get_skb_pptp_flag(struct sk_buff* skb)
{
	#if defined (FAST_PPTP)
	return skb->pptp_flag;
	#else
	return 0;
	#endif
}

void rtl_set_skb_pptp_flag(struct sk_buff* skb, unsigned int flag)
{
	#if defined (FAST_PPTP)
	skb->pptp_flag=flag;
	#endif
	return;
}

unsigned int rtl_get_skb_pptp_offset(struct sk_buff* skb)
{
	#if defined (FAST_PPTP)
	return skb->pptp_offset;
	#else
	return 0;
	#endif
}

void rtl_set_skb_pptp_offset(struct sk_buff* skb, unsigned int offset)
{
	#if defined (FAST_PPTP)
	skb->pptp_offset=offset;
	#endif
	return;
}

unsigned char *rtl_get_skb_data(struct sk_buff* skb)
{
	return skb->data;
}


void rtl_set_skb_data(struct sk_buff *skb, int offset, int action)
{
	if(action == 1)
		skb->data -= offset;
	else if(action == 0)
		skb->data += offset;

	return;
}


int rtl_skb_cloned(struct sk_buff *skb)
{
	return skb_cloned(skb);
}

unsigned int rtl_get_skb_len(struct sk_buff *skb)
{
	return skb->len;
}

void rtl_set_skb_dev(struct sk_buff *skb, struct net_device *dev)
{
	skb->dev = dev;
	return;
}

struct iphdr *rtl_ip_hdr(struct sk_buff *skb)
{
	return (struct iphdr *)(skb->data);
}

#endif

#if defined(FASTPTH_INDEPENDENCE_KERNEL)
struct dst_entry *dst_tmp = NULL;

/*As these API are used in fastpath, so skb will be check as valid, I will not check
skb again*/

unsigned int rtl_get_skb_len(struct sk_buff *skb)
{
	return skb->len;
}

__be16 rtl_get_skb_protocol(struct sk_buff *skb)
{
	return skb->protocol;
}

void rtl_set_skb_protocol(struct sk_buff *skb,__be16 protocol)
{
	skb->protocol=protocol;
}

unsigned char rtl_get_skb_type(struct sk_buff *skb)
{
	return skb->pkt_type;
}


__wsum rtl_get_skb_csum(struct sk_buff *skb)
{
	return skb->csum;
}


unsigned char *rtl_get_skb_data(struct sk_buff* skb)
{
	return skb->data;
}


void rtl_set_skb_data(struct sk_buff *skb, int offset, int action)
{
	if(action == 1)
		skb->data -= offset;
	else if(action == 0)
		skb->data += offset;

	return;
}

unsigned char *rtl_skb_mac_header(struct sk_buff * skb)
{
	return skb_mac_header(skb);
}

void rtl_skb_set_mac_header(struct sk_buff *skb, int offset)
{
	return skb_set_mac_header(skb, offset);
}


int rtl_skb_mac_header_was_set(struct sk_buff *skb)
{
	return skb_mac_header_was_set(skb);
}


void rtl_set_skb_dmac(struct sk_buff *skb, void *device)
{
	struct net_device *dev = (struct net_device *)device;

	memcpy(eth_hdr(skb)->h_dest, dev->dev_addr, ETH_ALEN);

	return;
}

void rtl_set_skb_smac(struct sk_buff *skb, void *device)
{
	struct net_device *dev = (struct net_device *)device;

	memcpy(eth_hdr(skb)->h_source, dev->dev_addr, ETH_ALEN);

	return;
}

unsigned char *rtl_skb_network_header(struct sk_buff * skb)
{
	return skb_network_header(skb);
}


void rtl_skb_set_network_header(struct sk_buff * skb,const int offset)
{
	skb_set_network_header(skb,offset);
}

void rtl_skb_reset_network_header(struct sk_buff *skb)
{
	return skb_reset_network_header(skb);
}

void rtl_set_skb_network_header(struct sk_buff * skb, unsigned char *network_header)
{
	skb->network_header=network_header;
}

unsigned char *rtl_skb_transport_header(struct sk_buff * skb)
{
	return skb_transport_header(skb);
}

void rtl_skb_set_transport_header(struct sk_buff * skb,const int offset)
{
	skb_set_transport_header(skb,offset);
}

void rtl_skb_reset_transport_header(struct sk_buff *skb)
{
	return skb_reset_transport_header(skb);
}

void rtl_set_skb_transport_header(struct sk_buff * skb, unsigned char *transport_header)
{
	skb->transport_header=transport_header;
}


unsigned char *rtl_skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull(skb,len);
}

unsigned char *rtl_skb_push(struct sk_buff *skb, unsigned int len)
{
	return skb_push(skb,len);
}


int rtl_ppp_proto_check(struct sk_buff *skb, unsigned char* ppp_proto)
{
	if(memcmp(skb->data-2, ppp_proto,2)==0)
		return 1;
	else
		return 0;
}

unsigned int rtl_ipt_do_table(struct sk_buff * skb, unsigned int hook, void *in, void *out)
{
	struct net_device *out_dev = (struct net_device *)out;
	struct net_device *in_dev;

	if(in == NULL)
		in_dev = skb->dev;
	else
		in_dev = (struct net_device *)in;

	return ipt_do_table(skb, hook, in_dev, out_dev, dev_net(skb->dev)->ipv4.iptable_mangle);
}

int rtl_ip_route_input(struct sk_buff  *skb, __be32 daddr, __be32 saddr, u8 tos)
{
	return ip_route_input(skb, daddr, saddr, tos, skb->dev);
}

int rtl_skb_dst_check(struct sk_buff *skb)
{
	int ret = SUCCESS;

	if( !(skb->dst->hh || skb->dst->neighbour)  ||skb->len > dst_mtu(skb->dst))
		ret = FAILED;

	return ret;
}

void rtl_set_skb_ip_summed(struct sk_buff *skb, int value)
{
	skb->ip_summed = value;
	return;
}

void rtl_dst_release(struct sk_buff *skb)
{
	dst_release(skb->dst);
	skb->dst = NULL;

	return;
}


__u32 rtl_get_skb_mark(struct sk_buff *skb)
{
	return skb->mark;
}

void rtl_set_skb_mark(struct sk_buff *skb, unsigned int value)
{
	skb->mark = value;

	return;
}


void rtl_store_skb_dst(struct sk_buff *skb)
{
	dst_tmp = skb->dst;

	skb->dst = NULL;

	return;
}

void rtl_set_skb_dst(struct sk_buff *skb)
{
	skb->dst = dst_tmp;

	return;
}

int rtl_tcp_get_timeouts(void *ptr)
{
	struct nf_conn *ct = (struct nf_conn *)ptr;

	return tcp_get_timeouts_by_state(ct->proto.tcp.state);
}

int rtl_arp_req_get_ha(__be32 queryIP, void *device, unsigned char * resHwAddr)
{
	struct net_device *dev = (struct net_device *)device;

	return arp_req_get_ha(queryIP, dev, resHwAddr);
}



u_int8_t rtl_get_ct_protonum(void *ct_ptr, enum ip_conntrack_dir dir)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	return ct->tuplehash[dir].tuple.dst.protonum;
}

unsigned long rtl_get_ct_udp_status(void *ct_ptr)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	return ct->status;
}

u_int8_t rtl_get_ct_tcp_state(void *ct_ptr)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	return ct->proto.tcp.state;
}

/*flag = 0 for src; flag = 1 for dst*/
__be32 rtl_get_ct_ip_by_dir(void *ct_ptr, enum ip_conntrack_dir dir, int flag)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	if(dir == IP_CT_DIR_ORIGINAL)
	{
		if(flag == 0)
			return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
		else if(flag == 1)
			return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
	}
	else if(dir == IP_CT_DIR_REPLY)
	{
		if(flag == 0)
			return ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip;
		else if(flag == 1)
			return ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip;
	}
}

/*flag = 0 for src; flag = 1 for dst*/
__be16 rtl_get_ct_port_by_dir(void *ct_ptr, enum ip_conntrack_dir dir, int flag)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	if(dir == IP_CT_DIR_ORIGINAL)
	{
		if(flag == 0)
			return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
		else if(flag == 1)
			return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;

	}
	else if(dir == IP_CT_DIR_REPLY)
	{
		if(flag == 0)
			return ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
		else if(flag == 1)
			return ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
	}
}

void rtl_set_ct_timeout_expires(void *ct_ptr, unsigned long value)
{
	struct nf_conn *ct = (struct nf_conn *)ct_ptr;

	ct->timeout.expires = value;

	return;
}

unsigned long rtl_hold_time(void *br_ptr)
{
	struct net_bridge *br = (struct net_bridge *)br_ptr;

	return br->topology_change ? br->forward_delay : br->ageing_time;
}

void rtl_set_fdb_aging(void *fdb_ptr, unsigned long value)
{
	struct net_bridge_fdb_entry *fdb = (struct net_bridge_fdb_entry *)fdb_ptr;

	fdb->ageing_timer = value;

	return;
}

unsigned long rtl_get_fdb_aging(void *fdb_ptr)
{
	struct net_bridge_fdb_entry *fdb = (struct net_bridge_fdb_entry *)fdb_ptr;

	return fdb->ageing_timer;
}

struct ethhdr *rtl_eth_hdr(struct sk_buff *skb)
{
	return eth_hdr(skb);
}


struct iphdr *rtl_ip_hdr(struct sk_buff *skb)
{
	return ip_hdr(skb);
}


struct net_device * rtl_get_dev_by_name(char *name)
{
	return __dev_get_by_name(&init_net, name);
}

struct net_device *rtl_get_skb_dev(struct sk_buff* skb)
{
	return skb->dev;
}


void rtl_set_skb_dev(struct sk_buff *skb, struct net_device *dev)
{
	if(dev == NULL)
		skb->dev = skb->dst->dev;
	else
		skb->dev = dev;

	return;
}

char *rtl_get_skb_dev_name(struct sk_buff *skb)
{
	return skb->dev->name;
}


void rtl_set_skb_inDev(struct sk_buff *skb)
{

	skb->inDev = skb->dev;

	return;
}

unsigned int rtl_get_skb_pppoe_flag(struct sk_buff* skb)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	return skb->pppoe_flag;
	#else
	return 0;
	#endif
}

void rtl_set_skb_pppoe_flag(struct sk_buff* skb, unsigned int pppoe_flag)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	skb->pppoe_flag=pppoe_flag;
	#endif
	return;
}

struct net_device *rtl_get_skb_rx_dev(struct sk_buff* skb)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	return skb->rx_dev;
	#else
	return NULL;
	#endif
}

void rtl_set_skb_rx_dev(struct sk_buff* skb,struct net_device *dev)
{
	#if defined (CONFIG_RTL_FAST_PPPOE)
	return skb->rx_dev=dev;
	#endif

	return;
}


char *rtl_get_ppp_dev_name(struct net_device *ppp_dev)
{
	return ppp_dev->name;
}

void * rtl_get_ppp_dev_priv(struct net_device *ppp_dev)
{
	return ppp_dev->priv;
}

int rtl_call_skb_ndo_start_xmit(struct sk_buff *skb)
{
	return skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
}


void rtl_inc_ppp_stats(struct ppp *ppp, int act, int len)
{

	if(act == 0){	//rx
		//ppp->stats.rx_packets ++;
		//ppp->stats.rx_bytes += len;
		ppp->dev->stats.rx_packets ++;
		ppp->dev->stats.rx_bytes += len;		
	}else if(act == 1){ //tx
		//ppp->stats.tx_packets ++;
		//ppp->stats.tx_bytes += len;
		ppp->dev->stats.tx_packets ++;
		ppp->dev->stats.tx_bytes += len;
	}
	return;
}

void *rtl_set_skb_tail(struct sk_buff *skb, int offset, int action)
{
	if(action == 1)
		skb->tail -= offset;
	else if(action == 0)
		skb->tail += offset;
	return;
}

struct sk_buff *rtl_ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb, int is_fast_fw)
{
	return ppp_receive_nonmp_frame(ppp, skb, is_fast_fw);
}

int rtl_ppp_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	return ppp_start_xmit(skb, dev);
}

void rtl_set_skb_cb(struct sk_buff *skb, char *value, int len)
{
	memcpy(skb->cb, value, len);

	return;
}

int rtl_ppp_vj_check(struct ppp* ppp)
{
	if(ppp->vj && !((ppp->xstate & SC_COMP_RUN) && ppp->xc_state))
		return 1;
	else
		return 0;
}

void *rtl_get_ppp_xmit_pending(struct ppp* ppp)
{
	return (void*)ppp->xmit_pending;
}

void rtl_set_ppp_xmit_pending(struct ppp* ppp, struct sk_buff* skb)
{
	ppp->xmit_pending = skb;

	return;
}

void rtl_set_skb_nfct(struct sk_buff *skb, void *value)
{
	skb->nfct = value;

	return;
}

struct neighbour *rtl_neigh_lookup(const void *pkey, struct net_device *dev)
{
	return neigh_lookup(&arp_tbl, pkey, dev);
}

struct hh_cache *rtl_get_hh_from_neigh(struct neighbour *neigh)
{
	return neigh->hh;
}

seqlock_t rtl_get_lock_from_hh(struct hh_cache * hh)
{
	return hh->hh_lock;
}

unsigned short rtl_get_len_from_hh(struct hh_cache * hh)
{
	return hh->hh_len;
}

unsigned long *rtl_get_data_from_hh(struct hh_cache * hh)
{
	return hh->hh_data;
}

unsigned int rtl_skb_headroom(struct sk_buff *skb)
{
	return skb_headroom(skb);
}

int rtl_skb_cloned(struct sk_buff *skb)
{
	return skb_cloned(skb);
}

int rtl_skb_shared(const struct sk_buff *skb)
{
	return skb_shared(skb);
}

void rtl_conntrack_drop_check_hook(struct nf_conn *ct_tmp, uint16 ipprotocol, struct nf_conn *ct)
{	
#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	if (ct_tmp->drop_flag == -1 && (ipprotocol == IPPROTO_TCP || ipprotocol == IPPROTO_UDP))
	{
		ct_tmp->drop_flag = __conntrack_drop_check(ct);
	}
#endif
}
#endif


int fast_path_before_nat_check(struct mbuf *skb, struct ip *iph, uint32 iphProtocol)
{
	#if defined(RTL_FP_CHECK_SPI_ENABLED) || defined(FAST_PATH_SPI_ENABLED)
	int	ret;
	#endif
	#if defined(FAST_PATH_SPI_ENABLED)
	unsigned int dataoff;
	if(fast_spi == 0)
		return NET_RX_PASSBY;
	#endif

	#if defined(FAST_PATH_SPI_ENABLED)
	if (iphProtocol== IPPROTO_TCP){
		dataoff = skb_network_offset(skb) + (iph->ihl<<2);
		ret = rtl_nf_conntrack_in(dev_net(skb->dev), dataoff, NF_INET_PRE_ROUTING, skb);
		switch (ret){
			case	NF_DROP:
				kfree_skb(skb);
				return NET_RX_DROP;
			case	NF_ACCEPT:
				break;
			default:
				kfree_skb(skb);
				return NET_RX_DROP;
			}
	}
	#endif

	return NET_RX_PASSBY;
}

int fast_path_post_process_xmit_check(struct ip *iph, struct tcphdr *tcphupuh, struct sk_buff *skb, struct ifnet *ifp)
{
	int ret;
	ret=NET_RX_SUCCESS;
	#if	defined(FAST_L2TP) || defined (CONFIG_RTL_FAST_PPPOE)
	if ((ret=enter_fast_path_fast_l2tp_post_process(skb,ifp))==NET_RX_DROP) 
	{
		return NET_RX_DROP;
	}
	else if ((ret=enter_fast_path_fast_pppoe_post_process(skb,ifp))==NET_RX_DROP)
	{		
		return NET_RX_DROP;
	}	
	else if ((ret=enter_fast_path_fast_pptp_post_process_skb(skb,ifp))==NET_RX_DROP)
	{		
		return NET_RX_DROP;
	}	
	else
	{		
		return ret;
	}
	#else
	return NET_RX_SUCCESS;
	#endif
}

int fast_path_post_process_xmit_check_mbuf(struct mbuf **mbuf, struct ifnet *ifp)
{
	int ret;
	ret=NET_RX_SUCCESS;
	#if	defined(FAST_PPTP)
	if ((ret = enter_fast_path_fast_pptp_post_process(mbuf,ifp)) == NET_RX_DROP)
	{
		return NET_RX_DROP;
	}
	else
	{		
		return ret;
	}
	#else
	return NET_RX_SUCCESS;
	#endif
}

int fast_path_post_process_return_check(struct ip *iph, struct tcphdr *tcphupuh, struct mbuf *m)
{
	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	if (iph->protocol==IPPROTO_UDP)
		return rtl_fp_gc_status_check(iph, tcphupuh, m);
	#endif

	return NET_RX_SUCCESS;

}

int32 rtl_fp_dev_queue_xmit_check(struct mbuf *skb)
{
	/*
	#ifdef FAST_L2TP
	if (l2tp_tx_id_hook != NULL)
		l2tp_tx_id_hook((void*)skb);
	#endif
	*/
	
	#if 0 //def FAST_PPTP
	if (sync_tx_pptp_gre_seqno_hook != NULL)
		sync_tx_pptp_gre_seqno_hook(skb);
	#endif

	return SUCCESS;
}

int32 rtl_fp_dev_hard_start_xmit_check(struct mbuf *skb)
{
/*
#if	defined(FAST_L2TP)
	#if	defined(CONFIG_NET_SCHED)
	if(!gQosEnabled)
	#endif
	if (l2tp_tx_id_hook != NULL)
		l2tp_tx_id_hook((void*)skb);
#endif
*/
	return SUCCESS;
}


int fastpath_ip_frag_output(struct ifnet *ifp, struct mbuf *m, 
	struct sockaddr *dst, struct rtentry *rt, struct ip *iph
#ifdef DUMMYNET
		,struct nat_info *natp
		,struct route* ro
#endif	
	)
{
	struct mbuf *m1;
	int left,error;
	u_short iphdr_len;
	u_short payloadlen;
	u_short frag_off;


	iphdr_len=(iph->ip_hl << 2);
	
	/*payload should algin on 8*/
	payloadlen=(rt->rt_ifp->if_mtu-iphdr_len) & (~7);
	
	frag_off = iph->ip_off |IP_MF ;
	left=iph->ip_len-payloadlen-iphdr_len;

	/*copy second frag*/
	m1 = m_copym2(m, 0, iphdr_len, M_NOWAIT);
	if(NULL == m1)
	{
		diag_printf("m_copym2 failed\n");
		m_freem(m);
		return -3;
		
	}
	memcpy(mtod(m1,u_char *)+iphdr_len,m->m_data+payloadlen+iphdr_len,left);


	/*1*/
	FASTPATH_ADJUST_CHKSUM_IPLEN_FRAG((payloadlen+iphdr_len), iph->ip_len, frag_off, iph->ip_off, iph->ip_sum);		
	iph->ip_len =(iphdr_len+ payloadlen);
	iph->ip_off |= IP_MF;
	m_adj(m,-left);
#if 0
	error=ip_finish_output3(rt->rt_ifp, m, dst, rt
				#ifdef DUMMYNET	
						, natp,	ro
				#endif
						);
#else //for code independence
	#if defined(DUMMYNET)
	error=ip_finish_output3(rt->rt_ifp, m, dst, rt, natp,ro);
	#else
	error=ip_finish_output3(rt->rt_ifp, m, dst, rt, NULL, NULL);
	#endif
#endif


	/*2*/
	iph=mtod(m1,struct ip *);
	m1->m_len +=(iphdr_len+left);
	if (m1->m_flags & M_PKTHDR)
		m1->m_pkthdr.len += (iphdr_len+left);
	frag_off = iph->ip_off;
	iph->ip_off+=(payloadlen>> 3);
	/*handle last frag or non-frag packet*/
	iph->ip_off &= (IP_MF | IP_OFFSET);
	FASTPATH_ADJUST_CHKSUM_IPLEN_FRAG((iphdr_len+left), iph->ip_len, iph->ip_off, frag_off, iph->ip_sum);
	iph->ip_len=(iphdr_len+left);
#if 0
	error=ip_finish_output3(rt->rt_ifp, m1, dst, rt
				#ifdef DUMMYNET	
					, natp, ro
				#endif
					);
#else
	#if defined(DUMMYNET) //for code independence
	error=ip_finish_output3(rt->rt_ifp, m1, dst, rt, natp, ro);
	#else
	error=ip_finish_output3(rt->rt_ifp, m1, dst, rt, NULL, NULL);
	#endif

#endif
	return error;
}

#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB
int _ip_finish_output3_skb(struct ifnet *ifp, struct sk_buff *skb,struct sockaddr *dst, struct rtentry *rt)
{
	short type;
	u_char edst[6];
	register struct ether_header *eh;
	struct net_device *dev;
	struct arpcom *ac = (struct arpcom *)(ifp);

	if((ifp==NULL) ||(skb==NULL)||(dst==NULL)||(rt==NULL)){
		if (skb){
			kfree_skb(skb);
		}
		return -1;
	}

#if 0
sc = (struct eth_drv_sc *) ifp->if_softc;
info = (Rltk819x_t *)sc->driver_private;
dev = (struct net_device *)info->dev;
return dev;

#endif
	//diag_printf("%s %d ifp->name (%s%d)\n",__FUNCTION__,__LINE__,ifp->if_name,ifp->if_unit);
	dev=rtl_getDevByIfp(ifp);
#if 0
	if((ifp->if_flags & IFF_UP) && (ifp->if_type == IFT_PPP))
#ifdef DUMMYNET
		return enter_qos_entry(ifp,m,natp,dst,ro);
#else
		return (*ifp->if_output)(rt->rt_ifp, m, dst, rt);
#endif
#endif
	
	/*Get ethernet header*/
	if(dst->sa_family == AF_INET){
		if (!arpget(ac, rt, dst, edst, rt))
		{
			kfree_skb(skb);
			//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			return -2;
		}	
		
		type = htons(ETHERTYPE_IP);
		//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
		rtl_skb_push(skb,sizeof(struct ether_header));
		eh = (struct ether_header *)(skb->data);
		memcpy(&eh->ether_type, &type, sizeof(eh->ether_type));
		memcpy(eh->ether_dhost, edst, sizeof (edst));
		memcpy(eh->ether_shost, ac->ac_enaddr, sizeof(eh->ether_shost));
		
		if(ifp->if_bridge) {	
			/*
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			diag_printf("da: %02x:%02x:%02x:%02x:%02x:%02x\n",edst[0],
				edst[1],edst[2],edst[3],edst[4],edst[5]);
			*/
			dev=getDevByMac(edst);
			if(!dev) {
				kfree_skb(skb);
				dump_fst_fdb();
				diag_printf("%s %d\n",__FUNCTION__,__LINE__);
				return -5;
			}
		}
/*
#ifdef FAST_L2TP
		if (fast_l2tp_fw)
			l2tp_tx_id(m);
#endif
*/
		/*Try to xmit the mbuf*/
		if (ifp->if_flags & IFF_UP){			
			//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			dev->hard_start_xmit(skb,dev);
			return 0;
		}else{
			kfree_skb(skb);
			return -3;
		}
	}else{
		kfree_skb(skb);
		return -4;
	}
}
#endif

int ip_finish_output3(struct ifnet *ifp, struct mbuf *m, 
	struct sockaddr *dst, struct rtentry *rt
#if 1//def DUMMYNET
	,struct nat_info *natp,struct route* ro
#endif
	)
{
	short type;
 	u_char edst[6];
	register struct ether_header *eh;
	struct arpcom *ac = (struct arpcom *)(ifp);

	if((ifp==NULL) ||(m==NULL)||(dst==NULL)||(rt==NULL)
#ifdef DUMMYNET
		||(natp==NULL)||(ro==NULL)
#endif
	)
		return -1;

	if((ifp->if_flags & IFF_UP) && (ifp->if_type == IFT_PPP)){
#ifdef DUMMYNET
		return enter_qos_entry(ifp,m,natp,dst,ro);
#else
		diag_printf("[%s,%d].fastpath if_output function call\n",__FUNCTION__,__LINE__);
		return (*ifp->if_output)(rt->rt_ifp, m, dst, rt);
#endif
	}

	/*Get ethernet header*/
	if(dst->sa_family == AF_INET){
		if (!arpresolve(ac, rt, m, dst, edst, rt))
			return -2;
		
		type = htons(ETHERTYPE_IP);

		M_PREPEND(m, sizeof (struct ether_header), M_DONTWAIT);
		if (m == 0)
			return ENOBUFS;
		eh = mtod(m, struct ether_header *);
		memcpy(&eh->ether_type, &type, sizeof(eh->ether_type));
		memcpy(eh->ether_dhost, edst, sizeof (edst));
		memcpy(eh->ether_shost, ac->ac_enaddr, sizeof(eh->ether_shost));
/*
#ifdef FAST_L2TP
		if (fast_l2tp_fw)
			l2tp_tx_id(m);
#endif
*/
		/*Try to xmit the mbuf*/
		if (ifp->if_flags & IFF_UP){
			
			#if 0
			if(ifp->if_bridge){			/*ifp is bridge interface*/
				bridge_output(ifp, m, NULL, NULL);
				return 0;
			}else{
				return (ifp->if_output)(rt->rt_ifp, m, dst, rt);
			}
			#endif
			extern int ether_output_frame(struct ifnet *ifp, struct mbuf *m);
#ifdef DUMMYNET			
			enter_qos_entry(ifp,m,natp,dst,ro);		
#else
			ether_output_frame(ifp, m);			
#endif
		}else{
			m_freem(m);
			return -3;
		}
	}else{
		m_freem(m);
		return -4;
	}
}

#if defined (CONFIG_RTL_FAST_PPPOE)
#ifdef __ECOS
#else
int ip_finish_output4(struct sk_buff *skb)
{
#if !defined(IMPROVE_QOS)
#if defined(CONFIG_NET_SCHED)
	if (gQosEnabled) {
		u_short proto = ntohs(skb->protocol);
		if(proto == ETH_P_IP){
			(list_empty(&nf_hooks[PF_INET][NF_IP_POST_ROUTING]))?: \
				ipt_do_table(skb, NF_IP_POST_ROUTING, skb->dev, NULL, \
				dev_net(skb->dev)->ipv4.iptable_mangle);
		}
	}
#endif
#endif

	if (skb->dev->flags & IFF_UP) 
	{
#if defined(CONFIG_NET_SCHED)
		if (gQosEnabled) 
		{
			// call dev_queue_xmit() instead of hard_start_xmit(), because I want the packets be sent through Traffic Control module
			dev_queue_xmit(skb);		
		}
		else            
#endif
		{

#if defined(CONFIG_BRIDGE)
			/*	In order to improve performance
			*	We'd like to directly xmit and bypass the bridge check
			*/
			if (skb->dev->priv_flags == IFF_EBRIDGE)
			{
				/*	wan->lan	*/
				struct net_bridge *br = netdev_priv(skb->dev);
				const unsigned char *dest = skb->data;
				struct net_bridge_fdb_entry *dst;

				if ((dst = __br_fdb_get(br, dest)) != NULL)
				{
					//skb->dev->stats.tx_packets++;
					//skb->dev->stats.tx_bytes += skb->len;
					skb->dev = dst->dst->dev;
				}
				else
				{
					kfree_skb(skb);
					return 0;
				}
			}
#endif
	
			skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
			skb->dev->stats.tx_packets++;
			skb->dev->stats.tx_bytes += skb->len;
			return 0;
		}

	}
	else
	{

		kfree_skb(skb);
	}
	
	return 0;
}
#endif
#endif
#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB
#define PPP_IPV4_PROTOCOL 0x0021
extern unsigned int fast_path_entry_count;
static int re865x_getIpv4Header_for_fastpath(uint8 *macFrame,struct ip **iph)
{
	uint8 *ptr;
	ptr=macFrame+12;
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
	{
		ptr=ptr+4;
	}
    if (*((uint16*)(ptr)) == htons(ETH_P_PPP_SES))
	{
			if(*(uint16 *)(ptr+2+6) == (uint16)htons(PPP_IPV4_PROTOCOL))
			{
				ptr = ptr+2+6+2;
                *iph=(struct iphdr *)(ptr);
				return 0;
			}
	}
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
{
		return -1;
	}
    *iph=(struct iphdr *)(ptr+2);
	return 0;
}
int FastPath_Enable(void)
{
	return (fast_nat_fw && get_fast_skb_fw());
}

int FastPath_Enable2(void)
{
	return (fast_nat_fw);
}

int FastPathSKB_Enter(struct net_device *dev, struct sk_buff  *skb, struct ether_header *eh)
{
	int ret;
#ifdef CONFIG_RTL_FREEBSD_FAST_PATH_SKB
	struct tcphdr *tcphupuh=NULL;
	struct ip *iph=NULL;
    re865x_getIpv4Header_for_fastpath(skb->data,&iph);
    if(iph&&(iph->ip_p != IPPROTO_GRE)){
		tcphupuh = (struct tcphdr*)((unsigned int *)iph + iph->ip_hl);
		//1723应该定义为PPTP_CONTROL_PORT宏
      if(tcphupuh&&(tcphupuh->th_sport!=htons(1701))&&(tcphupuh->th_sport!=htons(1723))&&!fast_path_entry_count)
            return 0;
        if(iph->ip_p == IPPROTO_TCP){
            if(((tcphupuh->th_flags & TH_RST) || (tcphupuh->th_flags & TH_SYN))){
			return 0;
		}
	}
    }
#endif
#ifdef CONFIG_RTL_PPPOE_DIRECT_REPLY
	if((magicNum != -1) && is_pppoe_lcp_echo_req(skb))
	{	
		if(direct_send_reply(skb)==1)
			return 1;							
	}
#endif
	
	if(0 == fast_nat_fw)
		return 0;
		
	#ifdef DUMMYNET	
	if(1 == get_QosEnabled())
		return 0;
	#endif

	if(1 == get_filterEnabled())
		return 0;
	
#if defined(CONFIG_RTL_FAST_PPPOE)
	check_and_pull_pppoe_hdr(skb);
#endif

	/*we just assume the skb packet coming in with ethernet header...
         Skip Header since skb fast path  want to see ipheader just point to skb->data...
         otherwise we need to deal with link layer header with different length in fastpath code*/
         
	rtl_skb_pull(skb,sizeof(struct ether_header));

#ifdef FAST_L2TP
	if (fast_l2tp_fw)
		fast_l2tp_rx(skb);
#endif

#ifdef FAST_PPTP
	if (fast_pptp_fw) {
		//fast_pptp_filter((void*)m,eh);
		fast_pptp_to_lan_skb(skb);
	}
#endif
	ret = enter_fast_path_skb(dev,skb,eh);

	/*if not hanlded. restore the packet*/
	if(ret!=NET_RX_DROP)
	{
#if defined(CONFIG_RTL_FAST_PPPOE)
		check_and_restore_pppoe_hdr(skb);	
#endif
#if defined(FAST_L2TP)
		check_and_restore_l2tp_hdr(skb);
#endif
#if defined(FAST_PPTP)
		check_and_restore_pptp_hdr(skb);
#endif
		rtl_skb_push(skb,sizeof(struct ether_header));
	}
	
	return ret;
}
#endif

int FastPath_Enter(struct ifnet *ifp, struct mbuf  **pm, struct ether_header *eh)
{
	int ret;
	struct mbuf *m;
	struct ip *iph=NULL;
	struct tcphdr *tcphupuh;  //just keep one , don't care tcp or udp //

	m=*pm;
	//skb->nh.raw = skb->data;
	//skb->transport_header=skb->data;
	//skb->network_header = skb->data;
	//skb_reset_network_header(skb);
//#if defined(CONFIG_RTL_FAST_PPPOE)
//	check_and_pull_pppoe_hdr(*pm);
//#endif

//hyking:
//bug fix:when port filter is enable,application will disable fast_nat_fw,at this moment,url filter is abnormal...
	iph = mtod(m, struct ip *);
	if(iph&&((iph->ip_p == IPPROTO_TCP)||(iph->ip_p == IPPROTO_UDP))){
		if(!fast_path_entry_count)
			return 0;
		tcphupuh = (struct tcphdr*)((unsigned int *)iph + iph->ip_hl);
		if(tcphupuh&&((tcphupuh->th_flags & TH_RST) || (tcphupuh->th_flags & TH_SYN))){
			return 0;
		}
	}
#if defined (DOS_FILTER) || defined (URL_FILTER)
		ret = filter_enter(m, ifp, eh);
		if (ret == NF_DROP) {
			//diag_printf("%s filter pkt, drop it\n", __FUNCTION__);
			m_freem(m);
			return 1;
		}
#if	defined(CONFIG_RTL_FAST_FILTER)
	else if(ret == NF_FASTPATH)
	{
		//continue the fastpath
	}
	else if(ret == NF_OMIT)
	{
		//don't support this in driver now
		ret=0;
		goto out;
	}
	else if(ret == NF_LINUX)
	{
		//don't do rtk fastpath
		ret=0;
		goto out;
	}
#endif
	else if(ret != NF_ACCEPT)
	{
		ret=0;
		goto out;
	}
#endif

	if (!fast_nat_fw)
	{
		ret=0;
		goto out;
	}

#ifdef FAST_PPTP 
	if (fast_pptp_fw) {
		fast_pptp_filter((void*)m,eh);
		#if 1
		ret = fast_pptp_to_lan((void*)&m);
		if (ret < 0)	// error, mbuf has been free!
		{
			return 1;
		}
		*pm=m;
		#endif
	}
#endif

/*
#ifdef FAST_L2TP
	if (fast_l2tp_fw)
		fast_l2tp_rx((void*)m);
#endif
*/

	ret = enter_fast_path(ifp, m, eh);

#if 0
	if(ret != NET_RX_DROP)
	{
		struct tcphdr *tcpudph;
		printk("-------%s(%d),ret(%d), src(0x%x),dst(0x%x), len is %d, version is %d\n",__FUNCTION__,__LINE__,ret,ip_hdr(skb)->saddr,ip_hdr(skb)->daddr, ip_hdr(skb)->ihl, ip_hdr(skb)->version);
		if(ip_hdr(skb)->protocol == IPPROTO_TCP)
		{
			tcpudph = (struct tcphdr*)((__u32 *)skb->data + ip_hdr(skb)->ihl);
			printk("===%s(%d),sport(%d),dport(%d),syn(%d),fin(%d),rst(%d)\n",__FUNCTION__,__LINE__,tcpudph->source,tcpudph->dest,tcpudph->syn,tcpudph->fin,tcpudph->rst);
		}
	}
#endif
	/*if not hanlded. restore the packet*/
#if 0 //def FAST_PPTP
	if (fast_pptp_fw && ret == 0 && (((struct ip*)m)->ip_p == IPPROTO_GRE) && m->m_pkthdr.len > sizeof(struct iphdr)&& pptp_tcp_finished==1)
		if(Check_GRE_rx_net_device(m))
		{
			fast_pptp_sync_rx_seq(m);
		}
#endif

out:
#if 0 //move to skb fast path
#if defined(CONFIG_RTL_FAST_PPPOE)
	if(ret!=NET_RX_DROP)
	{
		check_and_restore_pppoe_hdr(m);
	
	}
#endif
#endif
	return ret;
}

int smart_count=0;
unsigned long smart_count_start_timer;

/* return value:
	FAILED:			ct should be delete
	SUCCESS:		ct should NOT be delete.
*/
void rtl_delConnCache(struct alias_link *link)
{
	enum NP_PROTOCOL protocol;
	rtl_fp_napt_entry rtlFpNaptEntry;

	if (link->link_type == IPPROTO_TCP) {
		protocol = NP_TCP;
	} else if (link->link_type == IPPROTO_UDP) {
		protocol = NP_UDP;
	} else {
		return;
	}

	/*how to replay this lock?*/
	#ifndef __ECOS
	spin_lock_bh(&nf_conntrack_lock);
	#endif

	rtlFpNaptEntry.protocol = protocol;
	rtlFpNaptEntry.intIp = link->src_addr.s_addr;
	rtlFpNaptEntry.intPort = link->src_port;
	rtlFpNaptEntry.extIp = link->alias_addr.s_addr;
	rtlFpNaptEntry.extPort = link->alias_port;
	rtlFpNaptEntry.remIp = link->dst_addr.s_addr;
	rtlFpNaptEntry.remPort = link->dst_port;

	if(rtk_delNaptConnection(&rtlFpNaptEntry)==LR_SUCCESS)
		link->rtl_add_fastpath=0;

	#ifndef __ECOS
	spin_unlock_bh(&nf_conntrack_lock);
	#endif
}

#ifndef __ECOS
void rtl_check_for_acc(struct nf_conn *ct, unsigned long expires)
{
	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	int	newstate;
	struct list_head* state_hash;

	write_lock_bh(&nf_conntrack_lock);
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_TCP) {
		newstate = ct->proto.tcp.state;
		state_hash = Tcp_State_Hash_Head[newstate].state_hash;
	} else if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum == IPPROTO_UDP) {
		if(ct->status & IPS_SEEN_REPLY)
			newstate = 1;
		else
			newstate = 0;
		state_hash = Udp_State_Hash_Head[newstate].state_hash;
	} else {
		write_unlock_bh(&nf_conntrack_lock);
		return;
	}

	list_move_tail(&ct->state_tuple, state_hash);
	write_unlock_bh(&nf_conntrack_lock);
	#endif
}
#endif

int32 rtl_connCache_timer_update(struct alias_link *link)
{
	int idelta;
	long time_tmp;
	
	#ifndef __ECOS
	spin_lock_bh(&nf_conntrack_lock);
	#endif

	time_tmp = rtl_getTimeStamp();
	idelta = time_tmp - link->timestamp;
	if (idelta > link->expire_time) {
		if (SUCCESS==rtl_fpTimer_update(link)) {
			#ifndef __ECOS
			add_timer(&ct->timeout);
			spin_unlock_bh(&nf_conntrack_lock);
			#endif
			return SUCCESS;
		}
	}
	#ifndef __ECOS
	spin_unlock_bh(&nf_conntrack_lock);
	#endif
	return FAILED;
}

/*
 * ### for iperf application test ###
 * the behavior of iperf UDP test is LAN PC (client) will burst UDP from LAN to WAN (by one way),
 * WAN PC (server) will only send one UDP packet (statistics) at the end of test.
 * so the fastpath or hardware NAT will create link at the end of test.
 *
 * the purpose for adding the following code is to create fastpath or hardware NAT link
 * when we only get one packet from LAN to WAN in UDP case.
 */
int32 rtl_fpAddConnCheck(struct alias_link *link, struct ip *iph)
{
	struct tcphdr* tcph;
	uint32	sip, dip;
	int32	create_conn;

	sip = link->src_addr.s_addr;
	dip = link->dst_addr.s_addr;
	create_conn = FALSE;

	/*only for lan->wan can do this*/
	if ((link->dir == OUTBOUND_DIR) || (link->dir == INBOUND_DIR)
	#if defined(UNNUMBER_IP)
		 && (is_unnumber_ip(dip)==FALSE)
	#endif
	) {
		/* lan -> wan */
		if (iph->ip_p == IPPROTO_UDP &&
			(sip != _br0_ip) &&
			(((link->dir == OUTBOUND_DIR) &&((sip & _br0_mask) == (_br0_ip & _br0_mask)) &&
			((dip & _br0_mask) != (_br0_ip & _br0_mask))) ||
			((link->dir ==INBOUND_DIR) && ((sip & _br0_mask) == (_br0_ip & _br0_mask))&&
			((dip & _br0_mask) != (_br0_ip & _br0_mask))))&&
			(!IS_CLASSD_ADDR(dip))
			) {
			create_conn = TRUE;
			
			#ifndef __ECOS
			/* copied from last 2 line of this function **/
			set_bit(IPS_SEEN_REPLY_BIT, &ct->status);
			/* next 3 lines are copied from udp_packet() in ip_conntrack_proto_udp.c */
			nf_ct_refresh(ct,skb, nf_ct_udp_timeout_stream);
			/* Also, more likely to be important, and not a probe */
			set_bit(IPS_ASSURED_BIT, &ct->status);
			#endif
		} else if (iph->ip_p == IPPROTO_TCP) {
			tcph = (struct tcphdr *) ((char *) iph + (iph->ip_hl << 2));
			if (!(tcph->th_flags&TH_FIN) && !(tcph->th_flags&TH_SYN) && !(tcph->th_flags&TH_RST) && (tcph->th_flags&TH_PUSH) &&
				(tcph->th_flags&TH_ACK) &&  (iph->ip_dst.s_addr !=_br0_ip) &&
				(((link->dir == OUTBOUND_DIR) &&((sip & _br0_mask) == (_br0_ip & _br0_mask)) &&
				((dip & _br0_mask) != (_br0_ip & _br0_mask))) ||
				((link->dir ==INBOUND_DIR) && ((sip & _br0_mask) != (_br0_ip & _br0_mask))&&
				((dip & _br0_mask) == (_br0_ip & _br0_mask))))
				) {
				if (smart_count==0) {
					smart_count_start_timer = rtl_getTimeStamp()+1;
				}

				if (rtl_getTimeStamp() > smart_count_start_timer) {
					smart_count_start_timer = rtl_getTimeStamp()+1;
					smart_count=0;
				}

				smart_count++;
				if(smart_count >810){
					//panic_printk("the case hit for mart flow:tcp state=%d, assured=%d\n",ct->proto.tcp.state,test_bit(IPS_ASSURED_BIT, &ct->status));
					create_conn=TRUE;
				}
			}
		} else {
			return FALSE;
		}

#if defined(UNNUMBER_IP)
		if ((!create_conn)
			&& (is_unnumber_ip(sip)==TRUE))
			){
				create_conn = TRUE;
		}
#endif
	}

	return create_conn;
}
extern int rtl_urlfilter_check(struct ip *pip);
void rtl_fpAddConnCache(struct alias_link *link, struct ip *iph)
{
	int assured = 0;
	int create_conn = FALSE;
	enum NP_PROTOCOL protocol;
	rtl_fp_napt_entry rtlFpNaptEntry;

	if (!fast_nat_fw)
		return;

/*add by lq about nat loopback */	
	if (((link->src_addr.s_addr & _br0_mask) == (_br0_ip & _br0_mask))
		&& ((link->dst_addr.s_addr & _br0_mask) == (_br0_ip & _br0_mask))){
		return;	
	}
	assured = link->assured;
	if(!assured)
		create_conn = rtl_fpAddConnCheck(link, iph);
		
#if defined(CONFIG_RTL_BYPASS_PKT)
    if (link->bypass_cnt++ < RTL_BYPASS_PKT_NUM) {
        assured = 0;
        create_conn = 0;
    }
#endif
    if(rtl_urlfilter_check(iph))
    {
    	 assured = 0;
        create_conn = 0;
    }
	//diag_printf("assured is %d, link_type is %x, dir is %d, sIp is 0x%x, aIp is 0x%x, rIp is 0x%x\n", link->assured, link->link_type, link->dir,
		//	link->src_addr.s_addr, link->alias_addr.s_addr, link->dst_addr.s_addr);

	if (assured || (TRUE==create_conn))
	{
		if (link->link_type == IPPROTO_TCP) {
			protocol = NP_TCP;
		} else if (link->link_type == IPPROTO_UDP) {
			protocol = NP_UDP;
		}

		rtlFpNaptEntry.protocol = protocol;
		rtlFpNaptEntry.intIp = link->src_addr.s_addr;
		rtlFpNaptEntry.intPort = link->src_port;
		rtlFpNaptEntry.extIp = link->alias_addr.s_addr;
		rtlFpNaptEntry.extPort = link->alias_port;
		rtlFpNaptEntry.remIp = link->dst_addr.s_addr;
		rtlFpNaptEntry.remPort = link->dst_port;

		if(rtk_addNaptConnection(&rtlFpNaptEntry, NP_NONE) == LR_SUCCESS)
		{
			link->rtl_add_fastpath=1;
		}
	}

}
void rtl_skbfastpathOnOff(int value)
{
	fast_skb_fw = value;
}
void rtl_readSkbFastpath()
{
	diag_printf("skb fastpath %s\n", fast_skb_fw!=0?"Enabled":"Disabled");
}

void rtl_fastpathOnOff(int value)
{
	fast_nat_fw = value;
}
void rtl_showfastpathOnOff(void)
{
	diag_printf("fastpath %s\n", fast_nat_fw!=0?"Enabled":"Disabled");
}

#ifdef __ECOS
void rtl_getBrIpAndMask(unsigned long ip, unsigned long mask)
{
	_br0_ip		= ip;
	_br0_mask    = mask; 
	//printf("_br0_ip is 0x%x, _br0_mask is 0x%x\n", _br0_ip, _br0_mask);
}
#endif


//======================================
#ifdef __ECOS
int fastpath_init(void)
#else
static int __init fastpath_init(void)
#endif
{
	int			ret;
	int			buf[64];
	#ifdef CONFIG_FAST_PATH_MODULE
	fast_path_hook=FastPath_Enter;
	FastPath_hook1=rtk_delRoute;
	FastPath_hook2=rtk_modifyRoute;
	FastPath_hook3=rtk_addRoute;
	FastPath_hook4=rtk_delNaptConnection;
	FastPath_hook5=rtk_addArp;
	FastPath_hook6=rtk_addNaptConnection;
	FastPath_hook7=rtk_delArp;
	FastPath_hook8=rtk_modifyArp;
//	FastPath_hook9=Get_fast_pptp_fw;
//	FastPath_hook10=fast_pptp_to_wan;
	FastPath_hook11=rtk_idleNaptConnection;
	#endif

	#ifdef	DEBUG_PROCFILE
	/* proc file for debug */
	init_fastpath_debug_proc();
	#endif	/* DEBUG_PROCFILE */

	#ifndef NO_ARP_USED
	/* Arp-Table Init */
	ret=init_table_arp(ARP_TABLE_LIST_MAX,ARP_TABLE_ENTRY_MAX);
	if(ret!=0) {
		DEBUGP_SYS("init_table_arp Failed!\n");
	}
	#endif

	#ifndef DEL_ROUTE_TBL
	/* Route-Table Init */
	ret=init_table_route(ROUTE_TABLE_LIST_MAX, ROUTE_TABLE_ENTRY_MAX);
	if(ret!=0) {
		DEBUGP_SYS("init_table_route Failed!\n");
	}
	#endif

	#ifndef DEL_NAPT_TBL
	/* Napt-Table Init */
	ret=init_table_napt(NAPT_TABLE_LIST_MAX, NAPT_TABLE_ENTRY_MAX);
	if(ret!=0)
	{
		DEBUGP_SYS("init_table_napt Failed!\n");
	}
	#endif

	/* Path-Table Init */
	ret=init_table_path(PATH_TABLE_LIST_MAX, PATH_TABLE_ENTRY_MAX);
	if(ret!=0) {
		DEBUGP_SYS("init_table_path Failed!\n");
	}

	#ifdef CONFIG_UDP_FRAG_CACHE
	if(!udp_fragCache_init(MAX_UDP_FRAG_ENTRY))
		return -1;
	#endif

	#ifdef DOS_FILTER
	filter_init();
	#endif

	#if 0 //def FAST_PPTP
	fast_pptp_init();
	#endif

	#ifdef FAST_L2TP
	fast_l2tp_init();
	#endif
	
	#if defined (CONFIG_RTL_FAST_PPPOE)
	fast_pppoe_init();
	#endif

	#ifdef CONFIG_PROC_FS
	res1=create_proc_entry("fast_nat",0,NULL);
	if (res1) {
	    res1->read_proc=read_proc;
	    res1->write_proc=write_proc;
	}
	#endif


	#if defined(FAST_PATH_SPI_ENABLED)
	res_spi = create_proc_entry("fast_spi",0,NULL);
	if(res_spi){
		res_spi->read_proc = spi_read_proc;
		res_spi->write_proc = spi_write_proc;
	}
	#endif

	#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
	rtl_fp_gc_rx_threshold = RTL_FP_SESSION_LEVEL3_ALLOW_COUNT;
	#endif

	#ifdef __ECOS
	fp_jiffies = 0;
	#endif
	
	get_fastpath_module_info(buf);
	diag_printf("%s",buf);

	return 0;
}

#ifdef __ECOS
void fastpath_exit(void)
#else
static void __exit fastpath_exit(void)
#endif
{
#ifdef CONFIG_FAST_PATH_MODULE
	fast_path_hook=NULL;
	FastPath_hook1=NULL;
	FastPath_hook2=NULL;
	FastPath_hook3=NULL;
	FastPath_hook4=NULL;
	FastPath_hook5=NULL;
	FastPath_hook6=NULL;
	FastPath_hook7=NULL;
	FastPath_hook8=NULL;
	FastPath_hook9=NULL;
	FastPath_hook10=NULL;
	FastPath_hook11=NULL;
#endif

#ifdef	DEBUG_PROCFILE
	/* proc file for debug */
	remove_fastpath_debug_proc();
#endif	/* DEBUG_PROCFILE */

#ifdef DOS_FILTER
	filter_exit();
#endif

#if 0 //def FAST_PPTP
	fast_pptp_exit();
#endif

#if defined (CONFIG_RTL_FAST_PPPOE)
	fast_pppoe_exit();
#endif

#ifdef CONFIG_PROC_FS
	if (res1) {
		remove_proc_entry("fast_nat", res1);
		res1 = NULL;
	}
#endif

#if defined(FAST_PATH_SPI_ENABLED)
		if(res_spi){
			remove_proc_entry("fast_spi", res_spi);
			res_spi = NULL;
		}
#endif



	//printk("%s %s removed!\n", MODULE_NAME, MODULE_VERSION);
}

#if 1//defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
int rtl_mtu_check(unsigned int len, unsigned int mtu, uint32 frag_off)
{
	#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
	/*jwj: fastpath not kfree skb who need to do frag, but with don't frag bit set.*/
	if ((len > mtu) && (frag_off & IP_DF)) 
		return 1;
	else
		return 0;
	#else
	if (len > mtu)
		return 1;
	else
		return 0;
	#endif

}

int ip_finish_output3_skb(struct ifnet *ifp, struct sk_buff *skb,struct sockaddr *dst, struct rtentry *rt)
{
	int ret = 0;
	#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
	if (skb->len > ifp->if_mtu){
		
		ret = rtl_ip_fragment_for_fastpath(ifp, skb, dst, rt, _ip_finish_output3_skb);
		//dhcp/static ip, return value as 0 when forward success.
		if (ret == 1){
			ret = 0;
		}
	}
	else
	#endif
		ret = _ip_finish_output3_skb(ifp, skb, dst, rt);
	
	return ret;
}

#endif

#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
static void rtl_ip_copy_metadata(struct sk_buff *to, struct sk_buff *from)
{
#if defined(CONFIG_RTL_VLAN_SUPPORT)
	to->tag.v = from->tag.v;
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
    to->taged = from->taged;
    to->flag_src = from->flag_src;
    to->forward_rule = from->forward_rule;
    to->index = from->index;
#endif
#endif
#ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_set_wanorlan(&to->wanorlan,from->wanorlan);
#endif
#ifdef CONFIG_RTL_DELAY_REFILL
    to->priv = from->priv;
#endif

	to->dev = from->dev;

	return ;
}


void ip_options_fragment(struct sk_buff *skb)
{
	struct ip *iph = NULL;  
    unsigned char *cp = NULL;   
	int  l = 0;
	int  optlen = 0, opt_value = 0;

	if (!skb || !skb->data)
		return;
	
	iph = (struct ip *)(skb->data);
	l = (iph->ip_hl << 2) - sizeof (struct ip); 
	cp = (unsigned char *)((unsigned char *)iph + sizeof (struct ip) );

	while (l > 0) {
		opt_value = cp[IPOPT_OPTVAL];
		switch (opt_value) {
		case IPOPT_EOL:
			return;
		case IPOPT_NOP:
			l--;
			cp++;
			continue;
		}
		optlen = cp[IPOPT_OLEN];
		if (optlen < 2 || optlen > l)
		  return;
		if (!IPOPT_COPIED(*cp))
			memset(cp, IPOPT_NOP, optlen);
		l -= optlen;
		cp += optlen;
	}

	return;
}

void ip_send_check(struct ip *iph)
{
	iph->ip_sum = 0;
	iph->ip_sum = in_cksum_hdr(iph);
}

int rtl_ip_fragment_for_fastpath(struct ifnet *ifp, struct sk_buff *skb, struct sockaddr *dst, struct rtentry *rt, int (*output)(struct ifnet *, struct sk_buff *, struct sockaddr *, struct rtentry *))
{
	struct ip *iph = NULL;  
	int raw = 0;
	int ptr;
	//struct net_device *dev;
	struct sk_buff *skb2;
	unsigned int mtu, hlen, left, len, ll_rs, pad;
	int offset;
	__be16 not_last_frag;
	//struct rtable *rt = skb->rtable;
	int err = -1;

	//dev = rt->u.dst.dev;
	/*
	 *	Point into the IP datagram header.
	 */
	iph = (struct ip *)(skb->data);

	if ((iph->ip_off & htons(IP_DF))) {
		#if 0 
		//currently not support
		IP_INC_STATS(dev_net(dev), IPSTATS_MIB_FRAGFAILS);
		icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED,
			  htonl(dst_mtu(skb->dst)));
		#endif
		diag_printf("packets need fragment, but don't Fragment bit setted!\n");
		kfree_skb(skb);
		return -1;
	}
		/*
	 *	Setup starting values.
	 */

	hlen = iph->ip_hl * 4;
	mtu = ifp->if_mtu - hlen;	/* Size of data space */
	//IPCB(skb)->flags |= IPSKB_FRAG_COMPLETE;
	
	left = skb->len - hlen;		/* Space per frame */
	ptr = raw + hlen;		/* Where to start from */
	//diag_printf("%s %d iph->ip_off=%u hlen=%u mtu=%u skb->len=%u left=%u ptr=%d\n", __FUNCTION__, __LINE__, iph->ip_off, hlen, mtu, skb->len, left, ptr);

	/* for bridged IP traffic encapsulated inside f.e. a vlan header,
	 * we need to make room for the encapsulating header
	 */
	//pad = nf_bridge_pad(skb);
	pad = 0;//currently not support .
	//ll_rs = LL_RESERVED_SPACE_EXTRA(rt->u.dst.dev, pad);
	//ll_rs = ((4 * sizeof(struct ether_header) + pad) &~(HH_DATA_MOD - 1)) + HH_DATA_MOD;
	ll_rs =HH_DATA_RESERVE_LEN; 
	
	mtu -= pad;

	/*
	 *	Fragment the datagram.
	 */
	offset = (ntohs(iph->ip_off) & IP_OFFSET) << 3;
	not_last_frag = iph->ip_off & htons(IP_MF);
	//diag_printf("%s %d ll_rs=%u offset=%d not_last_frag=%u \n", __FUNCTION__, __LINE__, ll_rs, offset, not_last_frag);

	/*
	 *	Keep copying data until we run out.
	 */
	while (left > 0) {
		len = left;
		/* IF: it doesn't fit, use 'mtu' - the data space left */
		if (len > mtu)
			len = mtu;
		/* IF: we are not sending upto and including the packet end
		   then align the next start on an eight byte boundary */
		if (len < left)	{
			len &= ~7;
		}
		/*
		 *	Allocate buffer.
		 */

		if ((skb2 = alloc_skb(len+hlen+ll_rs)) == NULL) {
			diag_printf("IP: frag: no memory for new fragment!\n");
			err = -2;
			goto fail;
		}

		/*
		 *	Set up data on packet
		 */

		rtl_ip_copy_metadata(skb2, skb);
		skb_reserve(skb2, ll_rs);
		skb_put(skb2, len + hlen);
		//skb_reset_network_header(skb2);
		//skb2->transport_header = skb2->network_header + hlen;

		/*
		 *	Charge the memory for the fragment to any owner
		 *	it might possess
		 */
		
		//currently struct sk_buff not support skb->sk
		#if 0
		if (skb->sk){
			skb_set_owner_w(skb2, skb->sk);
		}
		#endif
		/*
		 *	Copy the packet header into the new buffer.
		 */

		//skb_copy_from_linear_data(skb, skb_network_header(skb2), hlen);
		memcpy(skb2->data, skb->data, hlen);

		/*
		 *	Copy a block of the IP datagram.
		 */
		//if (skb_copy_bits(skb, ptr, skb_transport_header(skb2), len))
			//BUG();
		//assume skb is linear data.
		memcpy(skb2->data+hlen, skb->data+ptr, len);
		
		left -= len;

		/*
		 *	Fill in the new header fields.
		 */
		iph = (struct ip *)(skb2->data);
		iph->ip_off = htons((offset >> 3));

		/* ANK: dirty, but effective trick. Upgrade options only if
		 * the segment to be fragmented was THE FIRST (otherwise,
		 * options are already fixed) and make it ONCE
		 * on the initial skb, so that all the following fragments
		 * will inherit fixed options.
		 */
		if (offset == 0)
			ip_options_fragment(skb);

		/*
		 *	Added AC : If we are fragmenting a fragment that's not the
		 *		   last fragment then keep MF on each bit
		 */
		if (left > 0 || not_last_frag)
			iph->ip_off |= htons(IP_MF);
		ptr += len;
		offset += len;

		/*
		 *	Put this fragment into the sending queue.
		 */
		iph->ip_len = htons(len + hlen);
		//diag_printf("%s %d iph->ip_len=%u len=%u, hlen=%u offset=%u\n", __FUNCTION__, __LINE__, iph->ip_len, len, hlen, offset);
		ip_send_check(iph);

		/*output can handle skb2, xmit it or kfree it, so do not need to care the
		value of err.*/
		err = output(ifp, skb2, dst, rt);
		//diag_printf("%s[%d], err is %d, id is %d\n", __FUNCTION__, __LINE__, err, iph->ip_id);
		//if (err == 0)
			//goto fail;
		
		//IP_INC_STATS(dev_net(dev), IPSTATS_MIB_FRAGCREATES);
	}
	kfree_skb(skb);
	//IP_INC_STATS(dev_net(dev), IPSTATS_MIB_FRAGOKS);
	return 1;
fail:
	kfree_skb(skb);
	//IP_INC_STATS(dev_net(dev), IPSTATS_MIB_FRAGFAILS);
	return err;
}
#endif


#ifndef __ECOS
module_init(fastpath_init);
module_exit(fastpath_exit);
MODULE_LICENSE("GPL");
#endif


/*
** Multicast Fast forward
*/
#if NBRIDGE > 0
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include <rtl/rtl865x_multicast.h>
extern int br0SwFwdPortMask;
#endif
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#define IP_VERSION4 4
#define IP_VERSION6 6

#define BR_MODULE	1
#define ETH_MODULE  0

#define IGMP_SNOOPING_ENABLE_FLAG 1
#define MLD_SNOOPING_ENABEL_FLAG 0

struct rtl_multicastDataInfo
{
	uint32 ipVersion;
	uint32 sourceIp[4];
	uint32 groupAddr[4];

};

struct rtl_multicastFwdInfo
{
	uint8 unknownMCast;
	uint8 reservedMCast;
	uint16 cpuFlag;
	uint32 fwdPortMask;
	
};

int ipMulticastFastFwd=1;
int needCheckMfc=1;

extern struct bridge_softc *rtl_getBridge(int brIndex);
#if defined(CONFIG_RTL_819X_SWCORE)
extern uint32 rtl_getIgmpSnoopingEnable(int snoopFlag);
 extern uint32 rtl_getIgmpModuleIndex(int moduleFlag);
 extern int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo);
 #endif

#if defined MROUTING
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
extern int rtl865x_checkMfcCache(unsigned long origin,unsigned long mcastgrp);
#endif
#endif

#if defined( CONFIG_RTL_HARDWARE_MULTICAST)

extern int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr);
extern int rtl865x_curOpMode;

#define MAX_UNKNOWN_MULTICAST_NUM 16
#define MAX_UNKNOWN_MULTICAST_PPS 1500
#define BLOCK_UNKNOWN_MULTICAST 1

struct rtl865x_unKnownMCastRecord
{
	unsigned int groupAddr;
	unsigned long lastJiffies;
	unsigned long pktCnt;
	unsigned int valid;
};
struct rtl865x_unKnownMCastRecord unKnownMCastRecord[MAX_UNKNOWN_MULTICAST_NUM];

int rtl865x_checkUnknownMCastLoading(struct rtl_multicastDataInfo *mCastInfo)
{
	int i;
	if(mCastInfo==NULL)
	{
		return 0;
	}
	/*check entry existed or not*/
	for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
	{
		if((unKnownMCastRecord[i].valid==1) && (unKnownMCastRecord[i].groupAddr==mCastInfo->groupAddr[0]))
		{
			break;
		}
	}

	/*find an empty one*/
	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
		{
			if(unKnownMCastRecord[i].valid!=1)
			{
				break;
			}
		}
	}

	/*find an exipired one */
	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		for(i=0; i<MAX_UNKNOWN_MULTICAST_NUM; i++)
		{
			if( time_before(unKnownMCastRecord[i].lastJiffies+HZ,jiffies)
				|| time_after(unKnownMCastRecord[i].lastJiffies,jiffies+HZ) )
			{
		
				break;
			}
		}
	}

	if(i==MAX_UNKNOWN_MULTICAST_NUM)
	{
		return 0;
	}

	unKnownMCastRecord[i].groupAddr=mCastInfo->groupAddr[0];
	unKnownMCastRecord[i].valid=1;
	
	if(time_after(unKnownMCastRecord[i].lastJiffies+HZ,jiffies))
	{
		unKnownMCastRecord[i].pktCnt++;
	}
	else
	{
		unKnownMCastRecord[i].lastJiffies=jiffies;
		unKnownMCastRecord[i].pktCnt=0;
	}

	if(unKnownMCastRecord[i].pktCnt>MAX_UNKNOWN_MULTICAST_PPS)
	{
		return BLOCK_UNKNOWN_MULTICAST;
	}

	return 0;
}
#endif

int rtl865x_ipMulticastFastFwd(struct ifnet *ifp, struct mbuf  *m, struct ether_header *eh)
{
	const unsigned char *dest = NULL;
	unsigned char *ptr;
	struct ip *iph=NULL;
	unsigned char proto=0;
	unsigned char reserved=0;
	int ret=-1;
	
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	struct sk_buff *skb2;
	
	uint32 IgmpModuleIndex=0xFFFFFFFF;
#if 0//defined (CONFIG_RTL_MLD_SNOOPING)
	struct ipv6hdr * ipv6h=NULL;
#endif
	unsigned int fwdCnt;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	unsigned int srcPort=0xFFFF;
	unsigned int srcVlanId=0;
	rtl865x_tblDrv_mCast_t *existMulticastEntry=NULL;
#endif
	struct bridge_softc *sc ;

	int brIndex=0;
	int s;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	srcPort= m->m_pkthdr.srcPort;
	srcVlanId= m->m_pkthdr.srcVlanId;
#endif
	/*check fast forward enable or not*/
	if(ipMulticastFastFwd==0)
	{
		return -1;
	}

	/*check dmac is multicast or not*/
	dest=(struct ether_addr *)&eh->ether_dhost[0];
	if((dest[0]&0x01)==0)
	{
		return -1;
	}
	//diag_printf("%s:%d,dest is 0x%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);

	 #if defined(CONFIG_RTL_819X_SWCORE)
	if(rtl_getIgmpSnoopingEnable(IGMP_SNOOPING_ENABLE_FLAG)==0)
	{
		return -1;
	}
	#endif
	sc = rtl_getBridge(brIndex);
	//sc = &bridgectl[0];

	/*check bridge0 exist or not*/
	if(sc==NULL) 
	{
		return -1;
	}

	
	if((ifp==NULL) ||((memcmp(ifp->if_xname,"eth",3)==0)&&(ifp->if_unit==0)))
	{
		return -1;
	}
	//diag_printf("%s:%d,dest is 0x%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);
	
	memset(&multicastFwdInfo,0,sizeof(struct rtl_multicastFwdInfo));
	/*check igmp snooping enable or not, and check dmac is ipv4 multicast mac or not*/
	if	((dest[0]==0x01) && (dest[1]==0x00) && (dest[2]==0x5e))
	{
		//diag_printf("%s:%d,eh->ether_type:%x,ifp:%s,%d \n",__FUNCTION__,__LINE__,eh->ether_type,ifp->if_xname,ifp->if_index);
		
	
		if( ntohs(eh->ether_type) != ETH_P_IP)
			return -1;

		iph = mtod(m, struct ip *);
		
		if(ntohl(iph->ip_dst.s_addr)== 0xEFFFFFFA)
		{
			/*for microsoft upnp*/
			reserved=1;
		}

		/*only speed up udp and tcp*/
		proto =  iph->ip_p;
		//diag_printf("%s:%d,proto is %d\n",__FUNCTION__,__LINE__,proto);
		if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)) && (reserved ==0))
		{
			multicastDataInfo.ipVersion=4;
			multicastDataInfo.sourceIp[0]=	(unsigned int)(iph->ip_src.s_addr);
			multicastDataInfo.groupAddr[0]=  (unsigned int)(iph->ip_dst.s_addr);

			multicastDataInfo.sourceIp[0] = ntohl(multicastDataInfo.sourceIp[0]);
			multicastDataInfo.groupAddr[0] = ntohl(multicastDataInfo.groupAddr[0]);

			#if defined MROUTING
			/*multicast data comes from wan, need check multicast forwardig cache*/
			if(((strncmp(ifp->if_name,"eth",3)==0) &&(ifp->if_unit==1))&& needCheckMfc )
			{
			#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
				int find=-1;
				find=rtl865x_checkMfcCache(iph->ip_src.s_addr, iph->ip_dst.s_addr);
				if( find == -1 )
			#endif	
				{

#if defined( CONFIG_RTL_HARDWARE_MULTICAST)

					if(rtl865x_checkUnknownMCastLoading(&multicastDataInfo)==BLOCK_UNKNOWN_MULTICAST)
#endif
					{
#if 0//defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)

						if((srcVlanId!=0) && (srcPort!=0xFFFF))
						{
							rtl865x_blockMulticastFlow(srcVlanId, srcPort, iph->ip_src.s_addr, iph->ip_dst.s_addr);
						}
						else
#endif
						{
							//diag_printf("free here!%s:%d,\n",__FUNCTION__,__LINE__);
							m_freem(m);
							return 0;
						}
					}
				
					return -1;
				}
			}
		#endif

			 #if defined(CONFIG_RTL_819X_SWCORE)
			IgmpModuleIndex = rtl_getIgmpModuleIndex(BR_MODULE);
			 ret= rtl_getMulticastDataFwdInfo(IgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			 #endif
			//diag_printf("%s:%d,ret:%d,dip: 0x%x,sip: 0x%x,multicastFwdInfo.fwdPortMask is 0x%x\n",__FUNCTION__,__LINE__,ret,(iph->ip_dst.s_addr),(iph->ip_src.s_addr),multicastFwdInfo.fwdPortMask);
			
			if((ret!=0)||multicastFwdInfo.reservedMCast || multicastFwdInfo.unknownMCast)
			{
				if( multicastFwdInfo.unknownMCast && 
					((strncmp(ifp->if_name,"eth",3)==0)&&(ifp->if_unit==1))
				#if defined( CONFIG_RTL_HARDWARE_MULTICAST)
					&& (rtl865x_checkUnknownMCastLoading(&multicastDataInfo)==BLOCK_UNKNOWN_MULTICAST)
				#endif
					)
				{
				//only block heavyloading multicast data from wan
#if 0//defined( CONFIG_RTL_HARDWARE_MULTICAST) || defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
					if((srcVlanId!=0) && (srcPort!=0xFFFF))
					{
						rtl865x_blockMulticastFlow(srcVlanId, srcPort, (unsigned int)(iph->ip_src.s_addr),(unsigned int)(iph->ip_dst.s_addr));
					}
					else
#endif
					{
						m_freem(m);
						return 0;
					}
				}
				return -1;
			}


			//diag_printf("%s:%d,br0SwFwdPortMask is 0x%x,multicastFwdInfo.fwdPortMask is 0x%x\n",__FUNCTION__,__LINE__,br0SwFwdPortMask,multicastFwdInfo.fwdPortMask);
			
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
			
			existMulticastEntry=rtl865x_findMCastEntry(multicastDataInfo.groupAddr[0], multicastDataInfo.sourceIp[0], srcVlanId, srcPort);
			if( (existMulticastEntry==NULL)||
				((existMulticastEntry!=NULL) && (existMulticastEntry->inAsic)))
			{
				
				if((srcVlanId!=0) && (srcPort!=0xFFFF))
				{
					/*multicast data comes from ethernet port*/
				#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
					if( (brSwFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
				#else
					if( (br0SwFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
				#endif
					{
						/*hardware forwarding ,let slow path handle packets trapped to cpu*/
						//diag_printf("hardware forwarding multicast.\n");
						return -1;
					}
				}
			}
#endif

			M_PREPEND(m, sizeof (struct ether_header), M_DONTWAIT);
            if (m == NULL)
				return (NULL);
            bcopy(eh, mtod(m, caddr_t), sizeof(struct ether_header));

			
			struct bridge_iflist *p;
			struct mbuf *mc;
			int used = 0;
			unsigned int port_bitmask=0;
			
			s = splimp();
			if(sc)
			{
				//diag_printf("from ifp:%s%d,%d.[%s]:[%d].\n",ifp->if_name,ifp->if_unit,ifp->if_index,__FUNCTION__,__LINE__);
			
				for (p = LIST_FIRST(&sc->sc_iflist); p; p = LIST_NEXT(p, next)) {
					//diag_printf("to ifp:port:%x,%x,%s,%d,%d.[%s]:[%d].\n",p->bif_port_id,p->bif_designated_port,p->ifp->if_name,p->ifp->if_unit,p->ifp->if_index,__FUNCTION__,__LINE__);
					/*
					 * Don't retransmit out of the same interface where
					 * the packet was received from.
					 */
					if (p->ifp->if_index == ifp->if_index)
						continue;
                    
					#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
                    if (should_deliver(p->ifp, m) == 0)
                        continue;
                    #endif
                    
					port_bitmask = (1 << p->ifp->if_index);
					if(!(port_bitmask & multicastFwdInfo.fwdPortMask))
						continue;
					
					//diag_printf("to ifp:%s%d,%d.[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_unit,p->ifp->if_index,__FUNCTION__,__LINE__);
					if (IF_QFULL(&p->ifp->if_snd)) {
						sc->sc_if.if_oerrors++;
						continue;
					}
				
					if (LIST_NEXT(p, next) == NULL) {
						used = 1;
						mc = m;
					} else {
						mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
						if (mc == NULL) {
							sc->sc_if.if_oerrors++;
							continue;
						}
					}
					sc->sc_if.if_opackets++;
					sc->sc_if.if_obytes += m->m_pkthdr.len;
								// Also count the bytes in the outgoing interface; normally
								// done in if_ethersubr.c but here we bypass that route.
								p->ifp->if_obytes += m->m_pkthdr.len;
					//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);			
					IF_ENQUEUE(&p->ifp->if_snd, mc);
					if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
						(*p->ifp->if_start)(p->ifp);
				}
				if (!used)
					m_freem(m);
			}
			splx(s);
			return (0);


		}

	}

#if 0//defined (CONFIG_RTL_MLD_SNOOPING)
	/*check igmp snooping enable or not, and check dmac is ipv4 multicast mac or not*/
	if	((dest[0]==0x33) && (dest[1]==0x33) && (dest[2]!=0xff))
	{
		
		if(mldSnoopEnabled==0)
		{
			return -1;
		}
		if( ntohs(eh->ether_type) != ETH_P_IPV6)
			return -1;
		
		struct ip6_hdr *ipv6h=NULL;
		ipv6h =(struct ip6_hdr*)mtod(m, struct ip6_hdr*);
		proto=re865x_getIpv6TransportProtocol(ipv6h);
			
		//printk("%s:%d,proto is %d\n",__FUNCTION__,__LINE__,proto);
		if((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
		{
			multicastDataInfo.ipVersion=6;
			memcpy(&multicastDataInfo.sourceIp, &ipv6h->ip6_src, sizeof(struct in6_addr));
			memcpy(&multicastDataInfo.groupAddr, &ipv6h->ip6_dst, sizeof(struct in6_addr));
			/*
			printk("%s:%d,sourceIp is %x-%x-%x-%x\n",__FUNCTION__,__LINE__,
				multicastDataInfo.sourceIp[0],multicastDataInfo.sourceIp[1],multicastDataInfo.sourceIp[2],multicastDataInfo.sourceIp[3]);
			printk("%s:%d,groupAddr is %x-%x-%x-%x\n",__FUNCTION__,__LINE__,
				multicastDataInfo.groupAddr[0],multicastDataInfo.groupAddr[1],multicastDataInfo.groupAddr[2],multicastDataInfo.groupAddr[3]);
			*/
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);
			if(ret!=0)
			{
				if(multicastFwdInfo.unknownMCast)
				{
					multicastFwdInfo.fwdPortMask=0xFFFFFFFF;
				}
				else
				{
					return -1;
				}

			}

			//printk("%s:%d,multicastFwdInfo.fwdPortMask is 0x%x\n",__FUNCTION__,__LINE__,multicastFwdInfo.fwdPortMask);

			M_PREPEND(m, sizeof (struct ether_header), M_DONTWAIT);
            if (m == NULL)
				return (NULL);
            bcopy(eh, mtod(m, caddr_t), sizeof(struct ether_header));
			
			struct bridge_iflist *p;
			struct mbuf *mc;
			int used = 0;
			unsigned int port_bitmask=0;
			struct bridge_softc *sc ;
			sc = &bridgectl[0];
			
			s = splimp();
			
			if(sc){
				//	diag_printf("from ifp:%s,%d.[%s]:[%d].\n",ifp->if_name,ifp->if_index,__FUNCTION__,__LINE__);
				for (p = LIST_FIRST(&sc->sc_iflist); p; p = LIST_NEXT(p, next)) {
					//diag_printf("to ifp:port:%x,%x,%s,%d.[%s]:[%d].\n",p->bif_port_id,p->bif_designated_port,p->ifp->if_name,p->ifp->if_index,__FUNCTION__,__LINE__);
					/*
					 * Don't retransmit out of the same interface where
					 * the packet was received from.
					 */
					if (p->ifp->if_index == ifp->if_index)
						continue;
					#if 1
					port_bitmask = (1 << p->ifp->if_index);
					if(!(port_bitmask & multicastFwdInfo.fwdPortMask))
						continue;
					#else
					if (memcmp(p->ifp->if_xname,"wlan",4)==0)
						continue;
					#endif
					//diag_printf("to ifp:%s,%d.[%s]:[%d].\n",p->ifp->if_name,p->ifp->if_index,__FUNCTION__,__LINE__);
					if (IF_QFULL(&p->ifp->if_snd)) {
						sc->sc_if.if_oerrors++;
						continue;
					}
				
					if (LIST_NEXT(p, next) == NULL) {
						used = 1;
						mc = m;
					} else {
						mc = m_copym(m, 0, M_COPYALL, M_NOWAIT);
						if (mc == NULL) {
							sc->sc_if.if_oerrors++;
							continue;
						}
					}
					sc->sc_if.if_opackets++;
					sc->sc_if.if_obytes += m->m_pkthdr.len;
								// Also count the bytes in the outgoing interface; normally
								// done in if_ethersubr.c but here we bypass that route.
								p->ifp->if_obytes += m->m_pkthdr.len;
					//diag_printf("[%s]:[%d].\n",__FUNCTION__,__LINE__);			
					IF_ENQUEUE(&p->ifp->if_snd, mc);
					if ((p->ifp->if_flags & IFF_OACTIVE) == 0)
						(*p->ifp->if_start)(p->ifp);
				}
			
				if (!used)
					m_freem(m);
			}
			splx(s);
			return (0);
		}

	}
#endif

	return -1;
}
#endif//CONFIG_RTL_IGMP_SNOOPING

#endif//NBRIDGE
int rtk_delNaptQosRuleEntry(struct Path_List_Entry *ep)
{
#if defined (CONFIG_RTL_QOS)
	struct ip_fw_l *q, *p;
#endif
	
#if defined (DUMMYNET)
	ep->flag = 0;					
#if defined (CONFIG_RTL_QOS)
	p=ep->rule;
	while(p)
	{
		q=p->next;
		free(p, M_IPFW);
		p=q;
	}
	ep->rule=NULL;
	ep->rule_nr=0;
#endif
#endif
}


int rtk_addNaptQosRuleEntry(struct Path_List_Entry *ep, struct nat_info *nat, unsigned int proto, char *if_name, short if_unit)
{
#ifdef DUMMYNET	
	if(get_QosEnabled())
	{
	
	#if	1
		/*let one udp packet go through protocol stack*/
		if(/*proto==IPPROTO_UDP||*/proto==IPPROTO_TCP)
			ep->flag |= FLAG_QOS_CHECK;
	#endif
	
		if(ep->flag&FLAG_QOS_CHECK)
		{
			if(ep->flag&FLAG_QOS_ENTER)
			{
		#if defined (CONFIG_RTL_QOS)
				nat->rule_nr =ep->rule_nr;
				nat->rule=ep->rule;
		#else
				nat->rule=ep->rule;
				nat->pipe_nr=ep->pipe_nr;
		#endif
			}
			else
			{
				lookup_qos_entry(nat,if_name,if_unit);
				/*sync rule and pipe info to nat entry*/
		#if defined (CONFIG_RTL_QOS)		
				ep->rule=nat->rule;
				ep->rule_nr=nat->rule_nr;
		#else
				ep->rule=nat->rule;
				ep->pipe_nr=nat->pipe_nr;
		#endif
				ep->flag |=FLAG_QOS_ENTER;
			}
		}
		else
		{
			ep->flag |= FLAG_QOS_CHECK;
			return 1;//need to return outside
		}	
	}	
#endif
	return 0;
}
int fastpath_get_QosEnabled(void)
{
#if defined (DUMMYNET)
	return get_QosEnabled();
#else
	return 0;
#endif
}

void fastpath_dump_qos_rule(struct Path_List_Entry *ep)
{
#if defined (DUMMYNET)&& defined (CONFIG_RTL_QOS)
	int i=0;
	struct ip_fw_l *rule=NULL;

	diag_printf("qosrule flag:%x,rule %d:",ep->flag,ep->rule_nr);
	rule=ep->rule;
	while(rule)
	{
		i++;
		
		diag_printf("[%d]	%d\n",rule->pipe_nr);
		rule=rule->next;
	}
	diag_printf("\n");
#endif
}
