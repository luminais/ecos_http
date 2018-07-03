/*utility.h*/
#ifndef __UTILITY_H__
#define __UTILITY_H__
#define SSID_LEN	32

#if defined(CONFIG_RTK_MESH) ||  defined(CONFIG_RTL_819X)  /*add for RTL819X since wlan driver default include mesh data*/
//by GANTOE for site survey 2008/12/26
#define MESHID_LEN 32 
#endif 

#define	MAX_BSS_DESC	64

#define _PATH_PROCNET_ROUTE	"/proc/net/route"
#define _PATH_PROCNET_DEV	"/proc/net/dev"
#ifdef ECOS_DBG_STAT
#include "../system/sys_utility.h"
extern int dbg_athttpd_index;
#endif

#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MBSSID
#endif



typedef struct countryIE {
	unsigned int		countryNumber;
	unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
	unsigned char 		countryName[24];
} COUNTRY_IE_ELEMENT;

typedef struct _OCTET_STRING {
    unsigned char *Octet;
    unsigned short Length;
} OCTET_STRING;


typedef enum _BssType {
    infrastructure = 1,
    independent = 2,
} BssType;


typedef	struct _IbssParms {
    unsigned short	atimWin;
} IbssParms;


typedef enum _Capability {
    cESS 		= 0x01,
    cIBSS		= 0x02,
    cPollable		= 0x04,
    cPollReq		= 0x01,
    cPrivacy		= 0x10,
    cShortPreamble	= 0x20,
} Capability;


typedef enum _Synchronization_Sta_State{
    STATE_Min		= 0,
    STATE_No_Bss	= 1,
    STATE_Bss		= 2,
    STATE_Ibss_Active	= 3,
    STATE_Ibss_Idle	= 4,
    STATE_Act_Receive	= 5,
    STATE_Pas_Listen	= 6,
    STATE_Act_Listen	= 7,
    STATE_Join_Wait_Beacon = 8,
    STATE_Max		= 9
} Synchronization_Sta_State;


typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;


typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#define	SECURITY_INFO_WAPI		0xa5a56789
#endif
typedef struct _BssDscr {
    unsigned char bdBssId[6];
    unsigned char bdSsIdBuf[SSID_LEN];
    OCTET_STRING  bdSsId;

#if defined(CONFIG_RTK_MESH) || defined(CONFIG_RTL_819X) 
	//by GANTOE for site survey 2008/12/26
	unsigned char bdMeshIdBuf[MESHID_LEN]; 
	OCTET_STRING bdMeshId; 
#endif 
    BssType bdType;
    unsigned short bdBcnPer;			// beacon period in Time Units
    unsigned char bdDtimPer;			// DTIM period in beacon periods
    unsigned long bdTstamp[2];			// 8 Octets from ProbeRsp/Beacon
    IbssParms bdIbssParms;			// empty if infrastructure BSS
    unsigned short bdCap;				// capability information
    unsigned char ChannelNumber;			// channel number
    unsigned long bdBrates;
    unsigned long bdSupportRates;		
    unsigned char bdsa[6];			// SA address
    unsigned char rssi, sq;			// RSSI and signal strength
    unsigned char network;			// 1: 11B, 2: 11G, 4:11G
	// P2P_SUPPORT
	unsigned char	p2pdevname[33];		
	unsigned char	p2prole;	
	unsigned short	p2pwscconfig;		
	unsigned char	p2paddress[6];	
	unsigned char	stage;	
    /*cfg p2p cfg p2p*/	
#if defined(WIFI_WPAS) || defined(RTK_NL80211)
    unsigned char	    p2p_ie_len;
    unsigned char	    p2p_ie[256];    
	unsigned char	    wscie_len;
	unsigned char	    wscie[256];    
    unsigned char	    wpa_ie_len;
    unsigned char	    wpa_ie[256];
    unsigned char	    rsn_ie_len;    
    unsigned char	    rsn_ie[256];
#endif	  

} BssDscr, *pBssDscr;
	// P2P_SUPPORT
enum p2p_role_s {
	R_P2P_GO =1	,
	R_P2P_DEVICE = 2,
	R_P2P_CLIENT =3  

};

typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    BssDscr bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;

typedef enum _wlan_wds_state {
    STATE_WDS_EMPTY=0, STATE_WDS_DISABLED, STATE_WDS_ACTIVE
} wlan_wds_state;

typedef struct _WDS_INFO {
	unsigned char	state;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	tx_errors;
	unsigned char	txOperaRate;
} WDS_INFO_T, *WDS_INFO_Tp;

struct _misc_data_ {
	unsigned char	mimo_tr_hw_support;
	unsigned char	mimo_tr_used;	
	unsigned char	resv[30];
};


typedef struct REG_DOMAIN_TABLE {
	unsigned char		region;
	unsigned int 		channel_set;
	unsigned char       area[24];
} REG_DOMAIN_TABLE_ELEMENT_T;

int web_write_chunked(char *fmt, ...);
void run_init_script(char *arg);
int fwChecksumOk(char *data, int len);
int getStats(char *interface, struct user_net_device_stats *pStats);
int htmlSpecialCharReplace(char*input,char* output,int bufLen);

#if defined(CONFIG_RTL_92D_SUPPORT)
void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB);
#endif
#endif
