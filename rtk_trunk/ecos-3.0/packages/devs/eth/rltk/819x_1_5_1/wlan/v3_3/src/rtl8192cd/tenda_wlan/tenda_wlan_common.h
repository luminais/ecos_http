#ifndef _TENDA_WLAN_COMMON_H_
#define _TENDA_WLAN_COMMON_H_

#ifndef MACSTR
#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifndef MAC2STR
#define MAC2STR(a)  (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

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

extern int mesh_extend_init(void);
extern void mesh_extend_fini(void);

#endif
