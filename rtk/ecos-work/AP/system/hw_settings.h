#ifndef HW_SETTINGS_H
#define HW_SETTINGS_H
/*
#define WIFI_SIMPLE_CONFIG
#define UNIVERSAL_REPEATER

#if !defined(CONFIG_RTL8196C_CLIENT_ONLY)
#define MBSSID
#endif

#define NUM_WLAN_INTERFACE	1	// number of wlan interface supported
#define NUM_WLAN_MULTIPLE_SSID	8	// number of wlan ssid support

#ifdef MBSSID 
#define NUM_VWLAN		4	// number of virtual wlan interface supported
#else
#define NUM_VWLAN		0
#endif

#ifdef UNIVERSAL_REPEATER
#define NUM_VWLAN_INTERFACE		NUM_VWLAN+1
#else
#define NUM_VWLAN_INTERFACE		NUM_VWLAN
#endif

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

#ifdef WIFI_SIMPLE_CONFIG
#define PIN_LEN					8
#endif

#define IFNAMSIZE       16

#define FLASH_BASE_ADDR			0xbd000000
#define HW_SETTING_OFFSET		0x6000
#define DEFAULT_SETTING_OFFSET		0x8000
#define CURRENT_SETTING_OFFSET		0xc000

#ifdef CONFIG_RTL_CODE_IMAGE_OFFSET
#define CODE_IMAGE_OFFSET               CONFIG_RTL_CODE_IMAGE_OFFSET
#else
#define CODE_IMAGE_OFFSET		0x20000
#endif
#ifdef CONFIG_RTL_WEB_PAGES_OFFSET
#define WEB_PAGE_OFFSET			CONFIG_RTL_WEB_PAGES_OFFSET
#else
#define WEB_PAGE_OFFSET			0x10000
#endif
#ifdef CONFIG_RTL_ROOT_IMAGE_OFFSET
#define ROOT_IMAGE_OFFSET		CONFIG_RTL_ROOT_IMAGE_OFFSET
#else
#define ROOT_IMAGE_OFFSET		0xE0000
#endif

#if !defined(MOVE_OUT_DEFAULT_SETTING_FROM_FLASH)
#define HW_SETTING_SECTOR_LEN		(0x8000-0x6000)
#define DEFAULT_SETTING_SECTOR_LEN	(0xc000-0x8000)
#define CURRENT_SETTING_SECTOR_LEN	(0x10000-0xc000)
#else
#define HW_SETTING_SECTOR_LEN		(0x8000-0x6000)
#define CURRENT_SETTING_SECTOR_LEN	(0x10000-0xc000)
#endif


#define __PACK__			__attribute__ ((packed))

// update tag
#define HW_SETTING_HEADER_TAG		((char *)"H6")
//#define DEFAULT_SETTING_HEADER_TAG	((char *)"6G")
#define DEFAULT_SETTING_HEADER_TAG	((char *)"6A")
//#define CURRENT_SETTING_HEADER_TAG	((char *)"6g")
#define CURRENT_SETTING_HEADER_TAG	((char *)"6a")


#define TAG_LEN				2

#define HW_SETTING_VER			1	// hw setting version
#define DEFAULT_SETTING_VER		1	// default setting version
#define CURRENT_SETTING_VER		DEFAULT_SETTING_VER // current setting version
		
#define SIGNATURE_LEN			4

typedef enum { FCC=1, IC, ETSI, SPAIN, FRANCE, MKK } REG_DOMAIN_T;

typedef struct hw_wlan_setting {
	unsigned char macAddr[6] __PACK__;
	unsigned char macAddr1[6] __PACK__;
	unsigned char macAddr2[6] __PACK__;
	unsigned char macAddr3[6] __PACK__;
	unsigned char macAddr4[6] __PACK__;
	unsigned char macAddr5[6] __PACK__;
	unsigned char macAddr6[6] __PACK__;
	unsigned char macAddr7[6] __PACK__;
	unsigned char pwrlevelCCK_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__; // CCK Tx power for each channel
	unsigned char pwrlevelCCK_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__; // CCK Tx power for each channel
	unsigned char pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM_MIB] __PACK__; 
	unsigned char pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiffHT20[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char pwrdiffOFDM[MAX_2G_CHANNEL_NUM_MIB] __PACK__;
	unsigned char regDomain __PACK__; // regulation domain
	unsigned char rfType __PACK__; // RF module type
	unsigned char ledType __PACK__; // LED type, see LED_TYPE_T for definition
	unsigned char xCap __PACK__; 
	unsigned char TSSI1 __PACK__; 
	unsigned char TSSI2 __PACK__; 
	unsigned char Ther __PACK__; 
	unsigned char trswitch __PACK__; 
	unsigned char trswpape_C9 __PACK__; 
	unsigned char trswpape_CC __PACK__; 
	unsigned char target_pwr __PACK__; 
	unsigned char Reserved5 __PACK__; 
	unsigned char Reserved6 __PACK__; 
	unsigned char Reserved7 __PACK__; 
	unsigned char Reserved8 __PACK__; 
	unsigned char Reserved9 __PACK__; 
	unsigned char Reserved10 __PACK__; 
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GHT20[MAX_5G_CHANNEL_NUM_MIB];
	unsigned char pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM_MIB];
//#ifdef WIFI_SIMPLE_CONFIG
	unsigned char wscPin[PIN_LEN+1] __PACK__;
//#endif

} HW_WLAN_SETTING_T, *HW_WLAN_SETTING_Tp;

typedef struct hw_setting {
	unsigned char boardVer __PACK__;	// h/w board version
	unsigned char nic0Addr[6] __PACK__;
	unsigned char nic1Addr[6] __PACK__;
	HW_WLAN_SETTING_T wlan[NUM_WLAN_INTERFACE];	
} HW_SETTING_T, *HW_SETTING_Tp;

typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN] __PACK__;  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;
*/


#define ETH_ALEN 6

#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MBSSID
#endif
char *read_hw_settings(void);
int modify_pin_code_of_hw_settings(char *pinbuf);
int modify_mac_addr_of_hw_settings(char *mac_address);
int write_hw_settings_to_default(void);
void dump_hw_settings(void);

#endif // HW_SETTINGS_H
