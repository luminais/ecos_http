#ifndef __WIFI_H__
#define __WIFI_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif


#define SSID_LENGTH_MAX 64

#define WIFI_DEFAULT_CHANNEL_NUM 13
#define OPEN		"0"
#define SHARED 		"1"
#define DISABLE		"disabled"
#define ENABLE		"enabled"
#define WL_24G          "wl0"
#define WL_24G_REPEATER "wl0.1"
#define WL_24G_GUEST       "wl0.2"
#define WL_24G_VA       "wl0.3"
#define WL_5G           "wl1"
#define WL_5G_REPEATER  "wl1.1"
#define WL_5G_GUEST        "wl1.2"
#define WL_5G_VA       "wl1.3"

//支持的功率等级,用于页面显示功率等级
#define SURPOT_HIGH_POWER    "no_hide"
#define SURPOT_NORMAL_POWER  "hide_normal"
#define SURPOT_LOW_POWER     "hide_power"

#define MAX_CHANNEL_NUM 76
/*
*由于Realtek里面密码错误和扫描中的桥接状态码不断的切换，
*为了保证页面获取密码错误状态不会变更，这里用STATUS_CHANGE_COUNT做了切换限制
*从密码错误到连接中再到密码错误状态一般情况下时间为3-5s，为了防止其他状态下可能切换耗时比较长，故这里定义为10s
*/
#define STATUS_CHANGE_COUNT 10 

#ifndef WIFI_SCAN_RETRY_TIMES
#define WIFI_SCAN_RETRY_TIMES 3
#endif

/*WIFI各个接口相关的配置参数*/
typedef struct wifi_ssid_cfg{
	PIU8 bss_enable;				  			  /* SSID开关 */
	PIU8 ssid_hide;				  				  /* SSID隐藏开关 */
	
	PI8 ssid[SSID_LENGTH_MAX + 1];				  /* SSID */	

	PI8 security[PI_BUFLEN_8];				      /* 加密模式， "none", "wep","wpapsk"*/
	
	PI8 wep_type[PI_BUFLEN_8];				      /* WEP加密类型， "open",  "shared"*/
	PIU8 wep_key;				      			  /* 当前WEP密钥， 1~4 */
	PI8 wep_key_arry[4][PI_BUFLEN_64+1];		  /* WEP密钥1-WEP密钥4 */
	
	PI8 wpa_psk_type[PI_BUFLEN_16];			  	  /* WPA个人加密类型，"psk", "psk2", "psk psk2" */
	PI8 wap_psk_crypto[PI_BUFLEN_16];			  /* WPA个人加密规则，"tkip", "aes", "tkip+aes" */
	PI8 wpa_psk_key[PI_BUFLEN_64+1];			  /* WPA个人密钥 */
}WIFI_SSID_CFG,*P_WIFI_SSID_CFG;

/* WIFI主配置参数*/
typedef struct wifi_info_struct{
	PIU8 wl_radio;				  		/*无线开关*/
	PI8 ifname[PI_BUFLEN_8];            /*无线接口*/
	
	WL_WORK_MODE work_mode;			  		/*ap,sta*/
	
	PI8 coutry_code[PI_BUFLEN_8];   	/*国家代码*/
	PI8 nettype[PI_BUFLEN_8]; 	  		/*b,g,bg,bgn,n only*/
	PIU8 channel; 	  				    /*0-14，信道*/
	PIU8 max_channel;					/*国家标准最大的信道*/
	PI8 bandwidth[PI_BUFLEN_8];  		/*20,40,auto，带宽*/
	PI8 bandside[PI_BUFLEN_8];	  		/*["none", "upper", "lower"]*/
	PI8 power[PI_BUFLEN_8];		  	    /*["high", "normal","low"]*/
	PI8 power_percent[PI_BUFLEN_16];	/*低中高的功率百分比*/
	
#ifdef __CONFIG_WPS_RTK__
	PI8 wps_enable[PI_BUFLEN_16];	    /* WPS开关 */
#endif

#ifdef __CONFIG_DISTURB_EN__
	PI8 antijam_en[PI_BUFLEN_8];		/*灵敏度开关*/
	PI8 antijam_cur_fa[PI_BUFLEN_8]; 
	PI8 antijam_cur_igi[PI_BUFLEN_8];			
#endif

	PIU8 wl_beamforming_en;				  		/*无线beamforming开关*/
	
	WIFI_SSID_CFG ap_ssid_cfg;			/*ap接口的配置*/

	WIFI_SSID_CFG guest_ssid_cfg;			/*访客接口的配置*/
	
	WIFI_SSID_CFG sta_ssid_cfg;			/*WISP和APCLIENT模式下连接上级接口的配置*/
}WIFI_INFO_STRUCT,*P_WIFI_INFO_STRUCT; 

/* WIFI主配置参数*/
typedef struct wifi_currcet_info_struct{	
	PIU8 channel; 	  				    /*1-14*/
	PIU8 bandwidth;  					/*20,40*/
}WIFI_CURRCET_INFO_STRUCT,*P_WIFI_CURRCET_INFO_STRUCT;


#define CHANNELS_1_11 0X0001
#define CHANNELS_1_13 0X0002
#define CHANNELS_36_48 0X0004
#define CHANNELS_149_161 0X0008
#define CHANNELS_149_165 0X0010

typedef struct country_info{
	char Country[16];
	int channels_24;
	int channels_5;
}COUNTRYINFO;


/*WIFI桥接接口参数*/
typedef struct wifi_bridge_info_struct{
	PIU8  rssi;
	PIU64 link_time;
}WIFI_BRIDGE_INFO_STRUCT,*P_WIFI_BRIDGE_INFO_STRUCT;

typedef enum
{
	RELAY_DISCONNECTED		= 1,
	RELAY_SUC					= 2,
	RELAY_AUTH_FAIL			= 3,
} WL_RELAY_STATUS;

typedef enum
{
	WLAN_START				= 1,
	WLAN_STOP					= 2,
	WLAN_RESTART				= 3,
} WL_OPERATOR;
    
#define TPI_CHANNEL_NUM_MAX		32 // number of channels in specific bandwidth  and band
typedef struct CHANNEL_ELEMENT{
	unsigned char region;
	unsigned char channel_num;
	unsigned char channels[TPI_CHANNEL_NUM_MAX];
}CHANNEL_ELEMENT_T;

/*API*/

/*GPI*/
extern P_WIFI_INFO_STRUCT gpi_wifi_get_info(WLAN_RATE_TYPE wl_rate);
extern void gpi_wifi_guest_get_info(P_WIFI_SSID_CFG cfg,char *ifname);
extern P_WIFI_CURRCET_INFO_STRUCT gpi_wifi_get_curret_info(WLAN_RATE_TYPE wl_rate,P_WIFI_CURRCET_INFO_STRUCT cfg);
RET_INFO gpi_wifi_channles_in_country();
void gpi_wifi_get_power_default(WLAN_RATE_TYPE wl_rate,PI8 *power);
void gpi_wifi_get_country_default(PI8 *country);

/*TPI*/
extern P_WIFI_INFO_STRUCT tpi_wifi_get_info(WLAN_RATE_TYPE wl_rate);
extern P_WIFI_CURRCET_INFO_STRUCT tpi_wifi_get_curret_info(WLAN_RATE_TYPE wl_rate,P_WIFI_CURRCET_INFO_STRUCT cfg);
PI32 tpi_wifi_get_channels(WLAN_RATE_TYPE wl_rate, PI32 bandwidth, PI8 *country, PIU16 *list, PI32 len);
PI32 tpi_wifi_get_channle_mun_by_country(PI8 *country);
void tpi_wifi_get_power_default(WLAN_RATE_TYPE wl_rate,PI8 *power);
void tpi_wifi_get_current_power(WLAN_RATE_TYPE wl_rate,PI8 *power);
void tpi_wifi_set_offset_power(WLAN_RATE_TYPE wl_rate);
void tpi_wifi_get_country_default(PI8 *country);
WL_WORK_MODE tpi_wifi_get_wl_work_mode(WLAN_RATE_TYPE rete_type);
extern RET_INFO tpi_wifi_update_info(WLAN_RATE_TYPE wl_rate);
extern RET_INFO tpi_wifi_struct_init();
extern RET_INFO tpi_wifi_first_init();
extern RET_INFO tpi_wifi_action(RC_MODULES_COMMON_STRUCT *var);
extern RET_INFO tpi_wifi_check_ip_confilct_by_wan(unsigned int wan_ip,unsigned int wan_mask);
extern SYS_WORK_MODE tpi_wifi_get_sys_work_mode();
#endif/*__WIFI_H__*/
