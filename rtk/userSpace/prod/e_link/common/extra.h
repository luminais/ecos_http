#ifndef ELINK_EXTRA_HEAD_
#define ELINK_EXTRA_HEAD_

#define ELINK_WAN_DISCONNECT (0)
#define ELINK_WAN_CONNECTED  (1)

#define MAX_ROW_LINE         256  

#define WEEK_MON (1)
#define WEEK_TUE (2)
#define WEEK_WED (3)
#define WEEK_THU (4)
#define WEEK_FRI (5)
#define WEEK_SAT (6)
#define WEEK_SUN (7)

#define WIFI_ENABLE (1)
#define WIFI_DISABLE (2)


#define MAX_STA_NUM  64
#define MAX_CLIENT_NUMBER 					255


/*************************************************

apidx_buf    0,1
enable_buf   yes,no
ssid_buf     
key_buf		 如果为为未加密的情况 none
auth_buf     wpapsk,wpa2psk,wpawpa2psk,none
encrypt_buf  若有auth加密认证则为aes,未加密为none

*************************************************/
#ifndef MIN
#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#endif

typedef struct elink_client{
	char mac[32];
	char vmac[32];
	int  connType;
}elink_client;

typedef struct ap_status
{
	int  apidx;
	char enable_buf[16];
	char ssid_buf[128];
	char key_buf[128];
	char auth_buf[64];
	char encrypt_buf[64];
	
}ap_status;
typedef enum 
{
	WIRED = 0,
	WIRESS_2G,
	WIRESS_5G,
}terminal_type;


typedef struct wlan_dev_probe_info_elink {  
    unsigned char               mac[18];
    int                         rssi;
    unsigned char               isAP;	
	int 					    network;
	char 						ssid[33];	
} WLAN_DEV_PROBE_INFO_ELINK_T, *WLAN_DEV_PROBE_INFO_ELINK_Tp;

struct net_device_elink_stats {
	unsigned long   rx_packets;           
	unsigned long   tx_packets;           
	unsigned long   rx_bytes;              
	unsigned long   tx_bytes;               
	unsigned long   rx_errors;             
	unsigned long   tx_errors;      
	unsigned long   tx_drops;
	unsigned long   rx_data_drops;
};

enum NETWORK_TYPE {
	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64
};
//extern int  extra_set_2g_ap_status_encrypt();
//int  extra_set_2g_ap_status_encrypt(int apidx,char *ap_status_encrypt)


#endif /* ELINK_EXTRA_HEAD_ */
