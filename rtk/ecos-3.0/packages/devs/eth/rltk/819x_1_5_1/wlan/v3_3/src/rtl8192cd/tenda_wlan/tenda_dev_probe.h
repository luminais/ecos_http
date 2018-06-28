#ifndef _TENDA_DEV_PROBE_ENABLE_H_
#define _TENDA_DEV_PROBE_ENABLE_H_
#include "tenda_wlan_common.h"

#define DEFAULT_DEV_PROBE_ENABLE    1

#define MACADDRLEN				    6
#define WLAN_SSID_MAXLEN	        32
#define DEV_PROBE_AGETIME           30      //30s
#define DEV_PROBE_CHECKTIMECNT      10      //5s
//#define BIT(x)      (1 << (x))
#define WLAN_HDR_A3_LEN		24
#define _ASOCREQ_IE_OFFSET_		4	// excluding wlan_hdr
#define	_ASOCRSP_IE_OFFSET_		6
#define _REASOCREQ_IE_OFFSET_	10
#define _REASOCRSP_IE_OFFSET_	6
#define _PROBEREQ_IE_OFFSET_	0
#define	_PROBERSP_IE_OFFSET_	12
#define _AUTH_IE_OFFSET_		6
#define _DEAUTH_IE_OFFSET_		0
#define _BEACON_IE_OFFSET_		12
#define _BEACON_CAP_OFFSET_	34

#define _HT_CAP_					45
#define _SSID_IE_				0
#define EID_VHTCapability			191	// Based on 802.11ac D2.0
#define _SUPPORTEDRATES_IE_		1
#define _EXT_SUPPORTEDRATES_IE_     50	// [802.11g 7.3.2] Extended supported rates
#define TRUE		1
#define FALSE		0

typedef struct dev_probe_info {
    unsigned char               use;                /*记录本列表信息是否被占用*/
    unsigned char               isAP;               /*记录设备是否是ap*/
    unsigned char               mac[MACADDRLEN];    /*记录设备mac*/
    unsigned long long               rx_time;            /*记录设备进入列表的时间*/
    int                         rssi;               /*记录设备信号强度*/
	int 					    network;            /*记录设备支持的网络类型*/
	char 						ssid[WLAN_SSID_MAXLEN+1];	
}dev_probe_info_t;
/*	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64*/
struct dev_probe_res {	    
    int     enable;
	int	    check_time_cnt;          /*每隔10s检查一下是否有超过30s没更新的设备*/
	int     dev_num;	             /*记录 2.4g 的设备个数*/
	int     dev_num_5g;	             /*记录 5g 的设备个数*/
	int     ext_num;	             /*记录 超出128个设备后，2.4g 的设备的存储序号*/        
	int     ext_num_5g;	             /*记录 超出128个设备后，5g 的设备的存储序号*/
	int     td_dev_probe_enable_old; /*记录 探针功能开关的前一次状态*/
    struct timer_list   age_timer;   
    int     age_timeout;             /*老化时长30s*/   
    unsigned char               dot11channel;       /*记录当2.4G前信道号*/
    unsigned char               dot11channel_5g;       /*记录当2.4G前信道号*/
};
#if 0
enum WIFI_FRAME_TYPE {
	WIFI_MGT_TYPE  =	(0),
	WIFI_CTRL_TYPE =	(BIT(2)),
	WIFI_DATA_TYPE =	(BIT(3)),
};

enum WIFI_FRAME_SUBTYPE {

	// below is for mgt frame
	WIFI_ASSOCREQ       = (0 | WIFI_MGT_TYPE),
    WIFI_ASSOCRSP       = (BIT(4) | WIFI_MGT_TYPE),
    WIFI_REASSOCREQ     = (BIT(5) | WIFI_MGT_TYPE),
    WIFI_REASSOCRSP     = (BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_PROBEREQ       = (BIT(6) | WIFI_MGT_TYPE),
    WIFI_PROBERSP       = (BIT(6) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_BEACON         = (BIT(7) | WIFI_MGT_TYPE),
    WIFI_ATIM           = (BIT(7) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DISASSOC       = (BIT(7) | BIT(5) | WIFI_MGT_TYPE),
    WIFI_AUTH           = (BIT(7) | BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DEAUTH         = (BIT(7) | BIT(6) | WIFI_MGT_TYPE),
    WIFI_WMM_ACTION		= (BIT(7) | BIT(6) | BIT(4) | WIFI_MGT_TYPE),
};
enum NETWORK_TYPE {
	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64
};
enum _RF_TX_RATE_ {
	_1M_RATE_	= 2,
	_2M_RATE_	= 4,
	_5M_RATE_	= 11,
	_6M_RATE_	= 12,
	_9M_RATE_	= 18,
	_11M_RATE_	= 22,
	_12M_RATE_	= 24,
	_18M_RATE_	= 36,
	_22M_RATE_	= 44,
	_24M_RATE_	= 48,
	_33M_RATE_	= 66,
	_36M_RATE_	= 72,
	_48M_RATE_	= 96,
	_54M_RATE_	= 108,
};
#endif
void dev_probe_proc_show(void *p);
#endif
