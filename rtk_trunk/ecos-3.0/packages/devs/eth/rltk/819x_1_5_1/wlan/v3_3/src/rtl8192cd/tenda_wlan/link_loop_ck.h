#ifndef _LINK_LOOP_CK_H_
#define _LINK_LOOP_CK_H_

#define DEFAULT_LINK_LOOP_ENABLE    1   //默认使能该功能

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN 6
#endif
#define MAX_DEV_NAME_LEN     16

enum {
    DEV_LAN_NAME = 0,
    DEV_BR_NAME,
    DEV_MESH_NAME,
    DEV_WLAN0_NAME,
    DEV_WLAN1_NAME,
    DEV_NUM_MAX
};

char s_dev_name[DEV_NUM_MAX][MAX_DEV_NAME_LEN] = {
    "eth0",
    "br0",
    "wlan-msh",
    "wlan0",
    "wlan1",
};

#define CW_DEV_GET_BY_NAME(dev)         __dev_get_by_name(&init_net, dev)

#define LINK_LOOP_MGMT_TYPE                  0x8197
#define LINK_LOOP_DATA_TYPE                  0x8198

#define LINK_LOOP_DEST_MAC                  "\xff\xff\xff\x21\x34\x5a"
#define LINK_LOOP_SEND_MSG_TIME                 1       //1s
#define LINK_LOOP_AGE_TIME                      3       //3S
#define LINK_LOOP_DEFAULT_MSG_LEN             1000

#define MAC_HASH_SIZE                           32

typedef struct mesh_node_info{
    unsigned char   lan_mac[MAC_ADDR_LEN];
    unsigned char   mesh_mac[MAC_ADDR_LEN];
    unsigned char   wlan0_mac[MAC_ADDR_LEN];
    unsigned char   wlan1_mac[MAC_ADDR_LEN];

    unsigned char	mesh_id[32];
    unsigned char   mesh_keys[65];    // passphrase

    //unsigned char   reserve[32];
}mesh_node_info_t;

struct mesh_node_table{
    spinlock_t			hash_lock;
    struct hlist_head   hash[MAC_HASH_SIZE];

};

struct mesh_node_list{
    struct hlist_node   hlist;
    unsigned char       mac[MAC_ADDR_LEN];
	int 				srcPhyPort;
    mesh_node_info_t    info;
    unsigned long       ack_jiffies;
};

typedef struct link_msg_data{
    unsigned short      len;
    mesh_node_info_t    node;
    unsigned int        check_sum;
} link_msg_data_t;

typedef struct link_loop_info {
    int                 enable;
    struct net_device   *dev_table[DEV_NUM_MAX];

    struct timer_list   send_msg_timer;
    link_msg_data_t     *msg_data;
    link_msg_data_t     *msg_crypto_data;
    struct net_device   *dev_wan;
} link_loop_info_t;



#endif
