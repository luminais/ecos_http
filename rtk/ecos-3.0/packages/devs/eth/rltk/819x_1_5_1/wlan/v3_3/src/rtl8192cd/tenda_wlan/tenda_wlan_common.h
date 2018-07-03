#ifndef _TENDA_WLAN_COMMON_H_
#define _TENDA_WLAN_COMMON_H_

#ifndef MACSTR
#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef MAC2STR
#define MAC2STR(a)  (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

#define MACADDRLEN				    6
#define WLAN_SSID_MAXLEN	        32
#define MAX_DEV_PROBE_NUM           128	
typedef struct tenda_dev_probe_ioctl {     
    unsigned char               mac[MACADDRLEN];    /*记录设备mac*/
    int                         rssi;               /*记录设备信号强度*/
    unsigned char               isAP;	            /*记录设备是否是ap*/
	int 					    network;            /*记录设备支持的网络类型*/
	char 						ssid[WLAN_SSID_MAXLEN+1];		
} web_tenda_dev_probe_ioctl; //BSPHZX++
#ifdef CONFIG_TENDA_WLAN_STA_STEER
typedef struct steer_bss_info {
    void                *priv;
    struct net_device   *dev;

    unsigned char       ssid[32];
    unsigned char       key_type;
    unsigned char       keys[65];
    unsigned char       ch;
    int                 band_steer_enable;
} steer_bss_info_t;

int sta_steer_check(void *priv,unsigned char *mac,unsigned char ch);
int sta_steer_collect_probe(void *priv, unsigned char *mac, int rssi, unsigned char ch);
void steer_update_bss_info(steer_bss_info_t *p, int down);
int sta_steer_assoc_roaming(void *priv, unsigned char channel, unsigned char *mac, int rssi);
void sta_steer_set_assoc(void *priv, unsigned char *mac, int assoc);
int sta_steer_get_auth_status();
int sta_steer_get_assoc_status();



int sta_steer_init(void);
void sta_steer_exit(void);
#endif

#ifdef CONFIG_TENDA_LINK_LOOP_CK
int link_loop_init(void);
void link_loop_exit(void);
//int _link_loop_tx_handle_(void *p);
int _ismac_in_link_list_(unsigned char *mac);
#endif

int dev_probe_init(void);
void dev_probe_exit(void);
void dev_probe_collect(unsigned char *mac, int rssi,unsigned char dot11channel, unsigned int pktlen, unsigned int frSubType, unsigned char *pframe,unsigned int ht_cap_elmt_size, unsigned int vht_cap_elmt_size);
int tenda_dev_probe_ioctl(unsigned char dot11channel, web_tenda_dev_probe_ioctl *p);
int tenda_dev_probe_num_ioctl(unsigned char dot11channel);
extern int mesh_extend_init(void);
extern void mesh_extend_fini(void);

#endif
