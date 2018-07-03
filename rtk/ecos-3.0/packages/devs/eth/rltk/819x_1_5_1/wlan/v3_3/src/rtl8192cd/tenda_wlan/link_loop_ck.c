/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ： tenda_sta_steering.c
Description : 用户迁移
Author ：jack deng
Version ：V1.0
Date ：2017-3-17
Others ：
History ：
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期： 修改者：
   内容：
2）...
 
*****************************************************************************/
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/time.h>
#include <linux/seq_file.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <linux/etherdevice.h>
#include <linux/crypto.h>
#include "link_loop_ck.h"
#include "tenda_wlan_common.h"

extern void rtl8192cd_get_mesh_id(struct net_device *dev,char *value,int len);
extern void rtl8192cd_get_mesh_keys(struct net_device *dev,char *value,int len);

static int s_link_loop_has_init = 0;
#if 0
#define DBG     printk
#else
#define DBG(x...)   do{} while(0)
#endif

static link_loop_info_t s_link_loop;
static struct mesh_node_table s_mesh_node_hash_table;

#define MESH_NODE_HASH_HEAD(mac) \
                &s_mesh_node_hash_table.hash[mac[5] & (MAC_HASH_SIZE - 1)]

static void mesh_node_list_init(void)
{
    memset(&s_mesh_node_hash_table, 0, sizeof(struct mesh_node_table));
    
}

static struct mesh_node_list *mesh_node_find_by_mac(unsigned char *mac)
{
    struct hlist_head       *head = MESH_NODE_HASH_HEAD(mac);
    struct mesh_node_list   *node;

    hlist_for_each_entry_rcu(node, head, hlist) {
        if (!memcmp(mac, node->mac, MAC_ADDR_LEN)) {
            return node;
        }
    }
    return NULL;
}

static int mesh_node_list_add(unsigned char *mac, mesh_node_info_t *p, int srcPhyPort)//(mesh_node_info_t *p)
{
    struct mesh_node_list  *node;

    node = mesh_node_find_by_mac(mac);
	if (!node) {
		node = (struct mesh_node_list *) kmalloc(sizeof(struct mesh_node_list), GFP_ATOMIC);
        if (node == NULL) {
            printk(KERN_ERR "%s: kmalloc failed\n", __FUNCTION__);
            return -1;
        }

		DBG("add new hash node\n");

		memcpy(&node->mac, mac,MAC_ADDR_LEN);
		hlist_add_head_rcu(&node->hlist, MESH_NODE_HASH_HEAD(mac));
	}

	memcpy(&node->info, p,sizeof(mesh_node_info_t));
    node->ack_jiffies = jiffies;
	node->srcPhyPort = srcPhyPort;

    return 0;
}

#if 0
static void mesh_node_list_del(unsigned char *mac)
{
    struct mesh_node_list  *node;

    node = mesh_node_find_by_mmac(mac);
    if (node) {
        hlist_del_rcu(&node->hlist);
        kfree(node);
    }
}
#endif

static void mesh_node_list_clear(void)
{
    struct mesh_node_table *table = &s_mesh_node_hash_table;
    struct mesh_node_list *node;
    struct hlist_node *n;
    int i;
    
    //lock
    for (i = 0; i < MAC_HASH_SIZE; i++) {
        hlist_for_each_entry_safe(node, n, &table->hash[i], hlist) {
            hlist_del_rcu(&node->hlist);
            kfree(node);
        }
    }
    //unlock
}

static void mesh_node_list_show(struct seq_file *s)
{
    struct mesh_node_table *table = &s_mesh_node_hash_table;
    struct mesh_node_list *node;
    int i, j;

	seq_printf(s,"===== MESH WIRED NEIGHBOR =====\n");
    seq_printf(s,"index peerMesh          rxport\n");
    for (i = 0, j = 0; i < MAC_HASH_SIZE; i++) {
        hlist_for_each_entry_rcu(node, &table->hash[i], hlist) {
			if (node->srcPhyPort >= 0) {
				seq_printf(s,"[%d]   "MACSTR" %u\n",
	                ++j, MAC2STR(node->info.lan_mac), node->srcPhyPort);
			}
        }
    }

	seq_printf(s,"\n===== HASH TABLE =====\n");
    seq_printf(s,"INDEX HASH  LAN-MAC           MESH-MAC           WLAN0-MAC        WLAN1-MAC        TIME\n");
    for (i = 0, j = 0; i < MAC_HASH_SIZE; i++) {
        hlist_for_each_entry_rcu(node, &table->hash[i], hlist) {
            seq_printf(s,"[%d]  0x%x   "MACSTR" "MACSTR" "MACSTR" "MACSTR" %lu\n",
                j++, node->mac[5], MAC2STR(node->info.lan_mac), MAC2STR(node->info.mesh_mac),
                MAC2STR(node->info.wlan0_mac), MAC2STR(node->info.wlan1_mac),
                (jiffies - node->ack_jiffies)/HZ);
        }
    }
}

static void mesh_node_list_age_handle(void)
{
    struct mesh_node_table *table = &s_mesh_node_hash_table;
    struct mesh_node_list *node;
    struct hlist_node *n;
    int i;
    
    //lock
    for (i = 0; i < MAC_HASH_SIZE; i++) {
        hlist_for_each_entry_safe(node, n, &table->hash[i], hlist) {
            if (jiffies > (node->ack_jiffies + LINK_LOOP_AGE_TIME*HZ)) {
                hlist_del_rcu(&node->hlist);
                kfree(node);
            }
        }
    }
    //unlock
}

int _ismac_in_link_list_(unsigned char *mac)
{
#if 0
    struct hlist_head       *head = MESH_NODE_HASH_HEAD(mac);
    struct mesh_node_list   *node;

    hlist_for_each_entry_rcu(node, head, hlist) {
        if (!memcmp(mac, node->info.mesh_mac, MAC_ADDR_LEN)) {
            return 1;
        }
    }
#endif
    return 0;
}

static unsigned short link_prepare_msg_hdr(unsigned char *frame,struct net_device *dev)
{
    unsigned short      len = 0;
    struct  ethhdr      *eh = (struct  ethhdr *)frame;

    /*Fill 802.3 head*/
    memcpy(eh->h_dest ,LINK_LOOP_DEST_MAC, 6);
    memcpy(eh->h_source ,dev->dev_addr , 6);
    eh->h_proto  = htons(LINK_LOOP_MGMT_TYPE);
    len = sizeof(struct  ethhdr);

    return len;
}
#ifdef CONFIG_CRYPTO_AES
static const char *s_aes_keys = "~B2@#sdSFM..@#$8cf32@#sa+--";
static void link_loop_data_decrypt(char *data, unsigned short len)
{
    int i;
    int blksize = 0;
    struct crypto_cipher *tfm;

    tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
    blksize = crypto_cipher_blocksize(tfm);
    crypto_cipher_setkey(tfm, s_aes_keys, strlen(s_aes_keys));

    for (i = 0; i < len; i += blksize) {
        crypto_cipher_decrypt_one(tfm, data + i, data + i);
    }
}

static void link_loop_data_encrypt(char *data, unsigned short len)
{
    int i;
    int blksize = 0;
    struct crypto_cipher *tfm;

    tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
    blksize = crypto_cipher_blocksize(tfm);
    crypto_cipher_setkey(tfm, s_aes_keys, strlen(s_aes_keys));
 
    for (i = 0; i < len; i += blksize) {
        crypto_cipher_encrypt_one(tfm, data + i, data + i);
    }
}
#else
static void link_loop_data_decrypt(char *data, unsigned short len)
{
    return;
}

static void link_loop_data_encrypt(char *data, unsigned short len)
{
    return;
}
#endif
static unsigned short link_update_msg_data(void)
{
    unsigned short      len = 0;
    link_loop_info_t    *link = &s_link_loop;
    link_msg_data_t     *msg = (link_msg_data_t *)(link->msg_data);
    mesh_node_info_t    *node = &msg->node;

    if (!msg) {
        return 0;
    }

    len = sizeof(link_msg_data_t);
    if (link->dev_table[DEV_LAN_NAME])
        memcpy(node->lan_mac, link->dev_table[DEV_LAN_NAME]->dev_addr, 6);

    if (link->dev_table[DEV_MESH_NAME])
        memcpy(node->mesh_mac, link->dev_table[DEV_MESH_NAME]->dev_addr, 6);

    if (link->dev_table[DEV_WLAN0_NAME]) {
        memcpy(node->wlan0_mac, link->dev_table[DEV_WLAN0_NAME]->dev_addr, 6);

        rtl8192cd_get_mesh_id(link->dev_table[DEV_WLAN0_NAME], node->mesh_id, sizeof(node->mesh_id));
        rtl8192cd_get_mesh_keys(link->dev_table[DEV_WLAN0_NAME], node->mesh_keys, sizeof(node->mesh_keys));
    }

    if (link->dev_table[DEV_WLAN1_NAME])
        memcpy(node->wlan1_mac, link->dev_table[DEV_WLAN1_NAME]->dev_addr, 6);

    msg->len = htons(len);
    msg->check_sum = 0;
    msg->check_sum = csum_partial(msg,len,0);

    memcpy(link->msg_crypto_data, link->msg_data, sizeof(link_msg_data_t));
    link_loop_data_encrypt((char *)link->msg_crypto_data, sizeof(link_msg_data_t));
    return len;
}


static void link_send_msg_timer_func(unsigned long data)
{
    link_loop_info_t    *link;
    struct net_device   *dev;
    struct sk_buff      *skb;
    unsigned short      off = 0;

    if (!data) {
        printk(KERN_ERR "%s:data = NULL\n",__func__);
        return;
    }
    
    link = (link_loop_info_t *)data;
    dev = link->dev_table[DEV_LAN_NAME];

    if (!link->enable) {
        goto out;
    }

    if (!dev) {
        printk(KERN_ERR "%s:lan_dev = NULL\n",__func__);
        goto out;
    }

    if (!(dev->flags & IFF_UP)) {
        goto out;
    }

    skb = dev_alloc_skb(LINK_LOOP_DEFAULT_MSG_LEN + 64);
    if (skb == NULL) {
        printk(KERN_ERR "%s:Unable to create package\n",__func__);
        goto out;
    }
    skb_reserve(skb, 64);
    memset(skb->data, 0, LINK_LOOP_DEFAULT_MSG_LEN);

    skb_reset_mac_header(skb);
    off = link_prepare_msg_hdr(skb->data, dev);
    skb_set_network_header(skb,off);

    memcpy(skb->data+off, link->msg_crypto_data, sizeof(link_msg_data_t));
    off += sizeof(link_msg_data_t);
    
    skb->len = off;
    skb->tail += off;
    skb->dev = dev;
    skb->protocol = htons(LINK_LOOP_MGMT_TYPE);
    //dev_queue_xmit(skb);
    if (link->dev_wan) {
        struct sk_buff *skb2 = skb_copy(skb, GFP_ATOMIC);
        if (skb2) {
            skb2->dev = link->dev_wan;
            dev_queue_xmit(skb2);
        }
    }
    dev->netdev_ops->ndo_start_xmit(skb, dev);
    mesh_node_list_age_handle();
out:
    mod_timer(&link->send_msg_timer, jiffies + LINK_LOOP_SEND_MSG_TIME*HZ);  
}
#if 0
static void link_age_timer_func(unsigned long data)
{
    link_loop_info_t    *link;

    if (!data) {
        printk(KERN_ERR "%s:data = NULL\n",__func__);
        return;
    }

    link = (link_loop_info_t *)data;
    mesh_node_list_age_handle();
    mod_timer(&link->age_timer, jiffies + LINK_LOOP_AGE_TIME*HZ); 
}
#endif
int _link_loop_tx_handle_(void *p, unsigned char *dst_wire, 
							unsigned char *sa_dst, unsigned char *sa_src)
{
    struct sk_buff      *skb = (struct sk_buff *)p;
    struct net_device   *dev = s_link_loop.dev_table[DEV_LAN_NAME];
    struct ethhdr       *eh;
 
    if (unlikely(dev == NULL)) {
        return -1;
    }
	
#if 1
    if (skb_headroom(skb) < 14) {   
        skb = skb_realloc_headroom(skb, 14);
        if (skb == NULL) {             
            return -1;
        }
    } else {            
        if (!(skb = skb_unshare(skb, GFP_ATOMIC))) {
            printk(KERN_ERR "%s: failed to unshare skbuff\n",__FUNCTION__);                   
            return -1;
        }
    }
#endif

	memcpy(skb->data, sa_dst, 6);
    memcpy(skb->data + 6, sa_src, 6);

    /* add ETH TYPE */
    eh = (struct ethhdr *)skb_push(skb, 14);
    memcpy(eh->h_dest , dst_wire/*LINK_LOOP_DEST_MAC*/, 6);
    memcpy(eh->h_source ,dev->dev_addr , 6);
    eh->h_proto  = htons(LINK_LOOP_DATA_TYPE);
	skb->dev = dev;

    dev->netdev_ops->ndo_start_xmit(skb, dev);
    return 0;
}

int mesh_tx_to_wire(struct sk_buff *skb, unsigned char *mesh_dst, unsigned char *mesh_nexthop, 
					unsigned char *sa_dst, unsigned char *sa_src)
{
	struct mesh_node_list   *node = NULL;
	unsigned char *dst = NULL;
//	int i;

	if (!s_link_loop.enable) {
		return -1;
	}

	if ((node = mesh_node_find_by_mac(mesh_dst)) != NULL) {
		/* 有线存在该Mesh链路的目的端，封装目的地地址为设备的有线接口地址 */
		dst = node->info.lan_mac;
//		printk("\nmesh data tx by wire, send to final bridge dst "MACSTR"", MAC2STR(dst));
	}
	else if ((node = mesh_node_find_by_mac(mesh_nexthop)) != NULL) {
		/* 有线存在该Mesh链路的下一跳，封装目的地地址为设备的Mesh接口地址  */
		dst = node->info.mesh_mac;
//		printk("\nmesh data tx by wire, send to next hop dst "MACSTR"", MAC2STR(dst));
	}
	else {
		return -1;
	}
/*
	printk("\nmest tx to wire, len=%d", skb->len);
	for (i = 0; i < skb->len; i++) {
		if (i%16 ==0)
			printk("\n");
		printk("%02x ", skb->data[i]);
	}
*/
	/* here, exitst wire nextop info */
	return _link_loop_tx_handle_(skb, dst, sa_dst, sa_src);
		
}

static int link_loop_is_same_mesh(link_msg_data_t *src, link_msg_data_t *dst)
{
    
    if (src->len != dst->len) {
        DBG("%s: src len(%d), dst len(%d)\n",__func__, src->len, dst->len);
        return 0;
    }

    if (strcmp(src->node.mesh_id, dst->node.mesh_id)) {
        DBG("%s: mesh id is not same\n",__func__);
        return 0;
    }

    if (strcmp(src->node.mesh_keys, dst->node.mesh_keys)) {
        DBG("%s: mesh keys is not same\n",__func__);
        return 0;
    }

    return 1;
}

int _link_loop_rx_handle_(struct sk_buff *skb, unsigned int srcPhyPort)
{
    struct  ethhdr      *eh = (struct  ethhdr *)(skb->data);
	struct net_device   *lan_dev = s_link_loop.dev_table[DEV_LAN_NAME];
	struct net_device   *mesh_dev = s_link_loop.dev_table[DEV_MESH_NAME];
	unsigned char 		*ptr = NULL;
    unsigned int        data_len = skb->len;

    DBG("RX: DEV(%s) len(%d) srcPhyPort(%d) type(0x%04x) htnos(0x%x)\n",
        skb->dev->name,skb->len,srcPhyPort,eh->h_proto,htons(LINK_LOOP_MGMT_TYPE));

	ptr = skb->data + 12;
	if (eh->h_proto == htons(ETH_P_8021Q)) {
		ptr += 4;
        data_len -= 4;
	}
	
    if (*(unsigned short *)(ptr) == htons(LINK_LOOP_MGMT_TYPE)) { //__constant_htons
    	ptr += 2;

        if (data_len >= 14 + sizeof(link_msg_data_t)) {
            link_msg_data_t msg;
            memcpy(&msg, ptr, sizeof(link_msg_data_t));
            link_loop_data_decrypt((char *)&msg, sizeof(link_msg_data_t));
            if (link_loop_is_same_mesh(&msg, s_link_loop.msg_data)) {
    	        mesh_node_list_add(msg.node.lan_mac, &msg.node, srcPhyPort);
    			mesh_node_list_add(msg.node.wlan0_mac, &msg.node, -1);
    			mesh_node_list_add(msg.node.wlan1_mac, &msg.node, -1);
            }  
        }

		dev_kfree_skb_any(skb);
        return 1;
		
    } 

	if (*(unsigned short *)(ptr) == htons(LINK_LOOP_DATA_TYPE)) {
    	/* 提取原始帧，初始化原始帧接口 */
		/*
    	printk("\nmesh rx by wire, len=%d", skb->len);
		for (i = 0; i < skb->len; i++) {
			if (i%16 ==0)
				printk("\n");
			printk("%02x ", skb->data[i]);
		}*/
    	if (!memcmp(lan_dev->dev_addr, eh->h_dest, 6)) {
			/* 如果目的地为本地, 则将Mesh设备口设为接入端，将原始帧送入内核 */
			skb_pull(skb, ptr - skb->data);
			eh = (struct  ethhdr *)(skb->data);
			/*
			printk("\nmesh rx by wire2, len=%d", skb->len);
			for (i = 0; i < skb->len; i++) {
				if (i%16 ==0)
					printk("\n");
				printk("%02x ", skb->data[i]);
			}*/
			#if 1  //br forward
			skb->dev = mesh_dev;
			skb->srcPhyPort = 88;//user for test
			skb->protocol = eth_type_trans(skb, mesh_dev);
			skb->ip_summed = CHECKSUM_NONE;			

		/*	
			printk("\nmesh data(dmac:"MACSTR" smac:"MACSTR" len=%d) rx form wire, send to kernel stack", 
				MAC2STR(eh->h_dest), MAC2STR(eh->h_source), skb->len);
		*/	
			//netif_rx(skb);
			netif_receive_skb(skb);
			#else  //shortcut forward
			if (eh->h_source[0] ==0xb0 && eh->h_dest[5] ==0x2b) {
				printk("\nrecv form mesh wire, fast tx, skip kernel stack");
				skb->dev = lan_dev;
				skb_reset_mac_header(skb);
				skb->protocol = eh->h_proto;
				skb->dev = lan_dev;
		//		skb->protocol = eth_type_trans(skb, br_dev);
				lan_dev->netdev_ops->ndo_start_xmit(skb, lan_dev);
			}
			#endif

			return 1;
    	}

		if (!memcmp(mesh_dev->dev_addr, eh->h_dest, 6)) {
			/* 如果本机为目的地下一跳, 则将Mesh设备口设备发送端，将原始帧发向Mesh设备口发往下一跳MP */
			printk("\nmesh data rx form wire, send to mesh dev");
			skb_pull(skb, ptr - skb->data);
			skb->dev = mesh_dev;
			mesh_dev->netdev_ops->ndo_start_xmit(skb, mesh_dev);

			return 1;
    	}
    }

#if 0   //shortcut
	if (skb->data[5] ==0x2b && skb->data[6] ==0xb0)
		printk("\n%s %d recv icmp request from lan dev, send to peer", __FUNCTION__, __LINE__);

	if (skb->data[0] ==0xb0 && skb->data[11] ==0x2b) {
		printk("\n%s %d recv icmp reponse from lan dev, fast tx to mesh dev", __FUNCTION__, __LINE__);

		skb_reset_mac_header(skb);
		eh = eth_hdr(skb);
		if (eh->h_proto == htons(ETH_P_8021Q)) {
			#if 1
		//		memcpy(skb->vlan_passthrough_saved_tag, skb->data+ETH_ALEN*2, 4);
				memmove(skb->data + 4, skb->data, ETH_ALEN<<1);
				skb_pull(skb, 4);
				eh = eth_hdr(skb);
			#else	
			printk("\ndon't handle vlan pkt, pass to kernel...");
			return 0;
			#endif
		}
		
		skb->protocol = eh->h_proto;
		skb->dev = mesh_dev;

		mesh_dev->netdev_ops->ndo_start_xmit(skb, mesh_dev);
		return 1;
	}
#endif		
	
    return 0;
}

static void link_loop_set_lan_dev(char *ifname)
{
    struct net_device   *dev;

    dev = __dev_get_by_name(&init_net, ifname);
    if (dev) {
        strncpy(s_dev_name[DEV_LAN_NAME],ifname, MAX_DEV_NAME_LEN - 1);
        s_link_loop.dev_table[DEV_LAN_NAME] = dev;
    } else {
        printk(KERN_ERR "%s: Can't find dev(%s)\n",__func__,ifname);
    }
}

static void link_loop_cfg_show(struct seq_file *s)
{
    link_loop_info_t *link = &s_link_loop;
    int i;

    seq_printf(s,"===== LINK LOOP CFG =====\n");
    seq_printf(s," enable: %d\n",link->enable);
    for (i = 0; i < DEV_NUM_MAX; i++) {
        seq_printf(s," %s: ",s_dev_name[i]);
        if (link->dev_table[i]) {
            seq_printf(s," "MACSTR"\n",MAC2STR(link->dev_table[i]->dev_addr));
        } else {
            seq_printf(s," Unkown dev!!!\n");
        }
    }

    if (link->dev_table[DEV_WLAN0_NAME]) {
        char value[128] = {0};
        rtl8192cd_get_mesh_id(link->dev_table[DEV_WLAN0_NAME],value,sizeof(value));
        seq_printf(s," MESH_ID  : %s\n",value);

        memset(value,0,sizeof(value));
        rtl8192cd_get_mesh_keys(link->dev_table[DEV_WLAN0_NAME],value,sizeof(value));
        seq_printf(s," MESH_KEYS: %s\n",value);
    }

    seq_printf(s," DEV_WAN  : %s\n",link->dev_wan ? link->dev_wan->name:"NULL");

    seq_printf(s,"\n");
}

static int link_loop_netdevice_event(struct notifier_block *this,
                                                    unsigned long event,
                                                    void *ptr)
{
    struct net_device *dev = ptr;
    int i;

    if (event == NETDEV_UP) {
        //printk("[%s] dev(%s) up\n",__func__,dev->name);
        for (i = 0; i < DEV_NUM_MAX; i++) {
            if (strcmp(s_dev_name[i], dev->name) == 0) {
                s_link_loop.dev_table[i] = dev;
                link_update_msg_data();
                break;
            }
        }
    }

    return NOTIFY_DONE;
}

static struct notifier_block link_loop_netdevice_notifier = {
    .notifier_call = link_loop_netdevice_event,
};


#define ROOT_DECLARE_READ_WRITE_PROC_FOPS(read_proc,write_proc) \
	static ssize_t read_proc##_write(struct file * file, const char __user * userbuf, \
		     size_t count, loff_t * off) \
	{ \
		return write_proc(file, userbuf,count, PDE_DATA(file_inode(file))); \
	} \
	int read_proc##_open(struct inode *inode, struct file *file) \
	{ \
			return(single_open(file, read_proc, PDE_DATA(file_inode(file)))); \
	} \
	struct file_operations read_proc##_fops = { \
			.open			= read_proc##_open, \
			.read			= seq_read, \
			.write			= read_proc##_write, \
			.llseek 		= seq_lseek, \
			.release		= single_release, \
	}

#define ROOT_CREATE_PROC_READ_WRITE_ENTRY(name, func, write_func, data) \
		proc_create_data(name, 0644, NULL, &func##_fops, (void *)data)

static int link_loop_read_proc(struct seq_file *s, void *data)
{
    link_loop_cfg_show(s);
    mesh_node_list_show(s);
    return 0;
}

static int link_loop_write_proc(struct file *file, const char *buffer,
                                    unsigned long count, void *data)
{
    char tmp[32];

    if (count > sizeof(tmp))
    	return -EINVAL;

	if (buffer && !copy_from_user(tmp, buffer, count)) {
        tmp[count-1] = '\0';
        if (count > 2) {
            if (strcmp(tmp, "wan_check_enable") == 0) {
                s_link_loop.dev_wan = __dev_get_by_name(&init_net, "eth1");
            } else if (strcmp(tmp, "wan_check_disable") == 0) {
                s_link_loop.dev_wan = NULL;
            } else {
                link_loop_set_lan_dev(tmp);
            }
        } else {
            s_link_loop.enable = simple_strtoul(tmp, NULL, 0);
            printk("=== set s_link_loop.enable(%d)\n",s_link_loop.enable);
        }
	}

    return count;
}

ROOT_DECLARE_READ_WRITE_PROC_FOPS(link_loop_read_proc,link_loop_write_proc);


int link_loop_init(void)
{
    link_loop_info_t *link = &s_link_loop;
    struct timer_list *timer;

    if (s_link_loop_has_init) {
        return 1;
    }

    link->msg_data = (link_msg_data_t *)kmalloc(sizeof(link_msg_data_t), GFP_ATOMIC);
    link->msg_crypto_data = (link_msg_data_t *)kmalloc(sizeof(link_msg_data_t), GFP_ATOMIC);

    if ((link->msg_data == NULL) || (link->msg_crypto_data == NULL)) {
        printk("%s: malloc msg data err\n",__func__);
        return 1;
    } else {
        memset(link->msg_data, 0, sizeof(link_msg_data_t));
        memset(link->msg_crypto_data, 0, sizeof(link_msg_data_t));
    }

#if 0
    //link->lan_dev = CW_DEV_GET_BY_NAME(LINK_LOOP_LAN_NAME);
    link->dev_table[DEV_LAN_NAME] = CW_DEV_GET_BY_NAME(s_dev_name[DEV_LAN_NAME]);
    if (link->dev_table[DEV_LAN_NAME] == NULL) {
        printk(KERN_ERR "%s:Can't find dev(%s)\n",__func__,s_dev_name[DEV_LAN_NAME]);
        return -1;
    }
#endif
    /* 发包定时器 */
    timer = &link->send_msg_timer;
    init_timer(timer);
    timer->function = link_send_msg_timer_func;
    timer->data = (unsigned long)link;
    timer->expires = jiffies + LINK_LOOP_SEND_MSG_TIME*HZ;
    add_timer(timer);
#if 0
    /* 老化定时器 */
    timer = &link->age_timer;
    init_timer(timer);
    timer->function = link_age_timer_func;
    timer->data = (unsigned long)link;
    timer->expires = jiffies + LINK_LOOP_AGE_TIME*HZ;
    add_timer(timer);
#endif
    register_netdevice_notifier(&link_loop_netdevice_notifier);

    mesh_node_list_init();
    ROOT_CREATE_PROC_READ_WRITE_ENTRY("link_loop",link_loop_read_proc,link_loop_write_proc,NULL);
    link->enable = DEFAULT_LINK_LOOP_ENABLE;
    s_link_loop_has_init = 1;
    printk("--- %s ---\n",__func__);
    return 0;
}

void link_loop_exit(void)
{
    link_loop_info_t *link = &s_link_loop;

    if (s_link_loop_has_init) {
        remove_proc_entry("link_loop",NULL);
        unregister_netdevice_notifier(&link_loop_netdevice_notifier);
        mesh_node_list_clear();
        //del_timer_sync(&link->age_timer);
        del_timer_sync(&link->send_msg_timer);
        kfree(link->msg_data);
        kfree(link->msg_crypto_data);
        s_link_loop_has_init = 0;
        printk("--- %s ---\n",__func__);
    }
}

EXPORT_SYMBOL(_link_loop_rx_handle_);