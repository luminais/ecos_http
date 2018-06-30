/*for form wlan handler*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#if 0
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>
#endif
#include <pkgconf/devs_eth_rltk_819x_wlan.h>

#include "athttpd.h"
#include "asp.h"
#include "fmget.h"
#include "apmib.h"
#include "common.h"
#include "utility.h"
#include "asp_form.h"
#include "sys_utility.h"
#include "sys_init.h"
#include "net_api.h"

//#define SDEBUG(fmt, args...) printf("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#define SDEBUG(fmt, args...) {}

#ifdef WIFI_SIMPLE_CONFIG
#define _WSC_DAEMON_PROG 	"wscd"
#endif

#ifdef MBSSID
#define DEF_MSSID_NUM RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#else
#define DEF_MSSID_NUM 0
#endif

#define WIFI_QUICK_REINIT

#ifdef WIFI_SIMPLE_CONFIG
#define START_PBC_MSG \
	"Start PBC successfully!<br><br>" \
	"You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define START_PIN_MSG \
	"Start PIN successfully!<br><br>" \
	"You have to run Wi-Fi Protected Setup in %s within 2 minutes."
#define SET_PIN_MSG \
	"Applied WPS PIN successfully!<br><br>" \
	"You have to run Wi-Fi Protected Setup within 2 minutes."
#define STOP_MSG \
	"Applied WPS STOP successfully!<br>"	
/*for WPS2DOTX brute force attack , unlock*/
#define UNLOCK_MSG \
	"Applied WPS unlock successfully!<br>"	
#endif

#ifdef HAVE_WPS	
extern  void forceWscStop(void);
#endif

static int wlan_idx_bak,vwlan_idx_bak;

static SS_STATUS_Tp pStatus=NULL;

#ifdef CONFIG_RTK_MESH
        #define _FILE_MESH_ASSOC "mesh_assoc_mpinfo"
        #define _FILE_MESH_ROUTE "mesh_pathsel_routetable"
		#define _FILE_MESH_ROOT  "mesh_root_info"
		#define _FILE_MESH_PROXY "mesh_proxy_table"
		#define _FILE_MESH_PORTAL "mesh_portal_table"		
		#define _FILE_MESHSTATS  "mesh_stats"
#endif // CONFIG_RTK_MESH


#ifdef WIFI_SIMPLE_CONFIG
enum {	CALLED_FROM_WLANHANDLER=1, CALLED_FROM_WEPHANDLER=2, CALLED_FROM_WPAHANDLER=3, CALLED_FROM_ADVANCEHANDLER=4, CALLED_FROM_CMJSYNCHANDLER=5};
struct wps_config_info_struct {
	int caller_id;
	int wlan_mode;
	int auth;
	int shared_type;
	int wep_enc;
	int wpa_enc;
	int wpa2_enc;
	unsigned char ssid[MAX_SSID_LEN];
	int KeyId;
	unsigned char wep64Key1[WEP64_KEY_LEN];
	unsigned char wep64Key2[WEP64_KEY_LEN];
	unsigned char wep64Key3[WEP64_KEY_LEN];
	unsigned char wep64Key4[WEP64_KEY_LEN];
	unsigned char wep128Key1[WEP128_KEY_LEN];
	unsigned char wep128Key2[WEP128_KEY_LEN];
	unsigned char wep128Key3[WEP128_KEY_LEN];
	unsigned char wep128Key4[WEP128_KEY_LEN];
	unsigned char wpaPSK[MAX_PSK_LEN+1];
};
struct wps_config_info_struct wps_config_info;
void update_wps_configured(int reset_flag);
#endif


WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};
WLAN_RATE_T rate_11n_table_80M_LONG[]={
	{MCS0, 	"29.3"},
	{MCS1, 	"58.5"},
	{MCS2, 	"87.8"},
	{MCS3, 	"117"},
	{MCS4, 	"175.5"},
	{MCS5, 	"234"},
	{MCS6, 	"263.3"},
	{MCS7, 	"292.5"},
	{MCS8, 	"58.5"},
	{MCS9, 	"117"},
	{MCS10, 	"175.5"},
	{MCS11, 	"234"},
	{MCS12, 	"351"},
	{MCS13, 	"468"},
	{MCS14, 	"526.5"},
	{MCS15, 	"585"},
	{0}
};
WLAN_RATE_T rate_11n_table_80M_SHORT[]={
	{MCS0, 	"32.5"},
	{MCS1, 	"65"},
	{MCS2, 	"97.5"},
	{MCS3, 	"130"},
	{MCS4, 	"195"},
	{MCS5, 	"260"},
	{MCS6, 	"292.5"},
	{MCS7, 	"325"},
	{MCS8, 	"65"},
	{MCS9, 	"130"},
	{MCS10, 	"195"},
	{MCS11, 	"260"},
	{MCS12, 	"390"},
	{MCS13, 	"520"},
	{MCS14, 	"585"},
	{MCS15, 	"650"},
	{0}
};

WLAN_RATE_T tx_fixed_rate[]={
	{1, "1"},
	{(1<<1), 	"2"},
	{(1<<2), 	"5.5"},
	{(1<<3), 	"11"},
	{(1<<4), 	"6"},
	{(1<<5), 	"9"},
	{(1<<6), 	"12"},
	{(1<<7), 	"18"},
	{(1<<8), 	"24"},
	{(1<<9), 	"36"},
	{(1<<10), 	"48"},
	{(1<<11), 	"54"},
	{(1<<12), 	"MCS0"},
	{(1<<13), 	"MCS1"},
	{(1<<14), 	"MCS2"},
	{(1<<15), 	"MCS3"},
	{(1<<16), 	"MCS4"},
	{(1<<17), 	"MCS5"},
	{(1<<18), 	"MCS6"},
	{(1<<19), 	"MCS7"},
	{(1<<20), 	"MCS8"},
	{(1<<21), 	"MCS9"},
	{(1<<22), 	"MCS10"},
	{(1<<23), 	"MCS11"},
	{(1<<24), 	"MCS12"},
	{(1<<25), 	"MCS13"},
	{(1<<26), 	"MCS14"},
	{(1<<27), 	"MCS15"},
	{((1<<31)+0), 	"NSS1-MCS0"},
	{((1<<31)+1), 	"NSS1-MCS1"},
	{((1<<31)+2), 	"NSS1-MCS2"},
	{((1<<31)+3), 	"NSS1-MCS3"},
	{((1<<31)+4), 	"NSS1-MCS4"},
	{((1<<31)+5), 	"NSS1-MCS5"},
	{((1<<31)+6), 	"NSS1-MCS6"},
	{((1<<31)+7), 	"NSS1-MCS7"},
	{((1<<31)+8), 	"NSS1-MCS8"},
	{((1<<31)+9), 	"NSS1-MCS9"},
	{((1<<31)+10), 	"NSS2-MCS0"},
	{((1<<31)+11), 	"NSS2-MCS1"},
	{((1<<31)+12), 	"NSS2-MCS2"},
	{((1<<31)+13), 	"NSS2-MCS3"},
	{((1<<31)+14), 	"NSS2-MCS4"},
	{((1<<31)+15), 	"NSS2-MCS5"},
	{((1<<31)+16), 	"NSS2-MCS6"},
	{((1<<31)+17), 	"NSS2-MCS7"},
	{((1<<31)+18), 	"NSS2-MCS8"},
	{((1<<31)+19), 	"NSS2-MCS9"},
	{0}
};

//changes in following table should be synced to VHT_MCS_DATA_RATE[] in 8812_vht_gen.c
static const unsigned short VHT_MCS_DATA_RATE[3][2][20] = 
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312},			// Long GI, 20MHz
			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			29, 58, 87, 116, 173, 231, 260, 289, 347, 347}	},		// Short GI, 20MHz
		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360, 
			54, 108, 162, 216, 324, 432, 486, 540, 648, 720}, 		// Long GI, 40MHz
			{30, 60, 90, 120, 180, 240, 270, 300,360, 400, 
			60, 120, 180, 240, 360, 480, 540, 600, 720, 800}},		// Short GI, 40MHz
		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560}, 	// Long GI, 80MHz
			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867, 
			130, 260, 390, 520, 780, 1040, 1170, 1300, 1560,1733}	}	// Short GI, 80MHz
	};

/////////////////////////////////////////////////////////////////////////////

#if defined(CONFIG_RTL_92D_SUPPORT)
static int _isBandModeBoth()
{
	int val;
	apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&val);
	if(val == BANDMODEBOTH)
		return 1;
	else
		return 0;
}
#endif

#if defined(HAVE_TR069)
void apmib_save_wlanIdx(void)
{
        wlan_idx_bak    =       apmib_get_wlanidx();
        vwlan_idx_bak   =       apmib_get_vwlanidx();
}
void apmib_recov_wlanIdx(void)
{
        apmib_set_wlanidx(wlan_idx_bak);
        apmib_set_vwlanidx(vwlan_idx_bak);
}
#endif

static void _Start_Wlan_Applications(void)
{

	#if defined (CONFIG_RTL_92D_SUPPORT)
	if(_isBandModeBoth())
		system("sysconf wlanapp start wlan0 wlan1 br0");
	else
		system("sysconf wlanapp start wlan0 br0");
	#else
	system("sysconf wlanapp start wlan0 br0");
	#endif
	sleep(1);
	/*sysconf upnpd 1(isgateway) 1(opmode is bridge)*/
	system("sysconf upnpd 1 1");
	sleep(1);
}

static inline int isAllStar(char *data)
{
	int i;
	for (i=0; i<strlen(data); i++) {
		if (data[i] != '*')
			return 0;
	}
	return 1;
}

#ifdef WIFI_SIMPLE_CONFIG
#if 0
#ifndef WLAN_EASY_CONFIG
void sigHandler_autoconf(int signo)
{
	#define REINIT_WEB_FILE		("/tmp/reinit_web")
	struct stat status;
	int reinit=1;

	if (stat(REINIT_WEB_FILE, &status) == 0) { // file existed
        	unlink(REINIT_WEB_FILE);
		reinit = 0;		
	}	
	if (reinit) { // re-init system
#ifdef REBOOT_CHECK
	run_init_script_flag = 1;
#endif		
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Start_Domain_Query_Process=0;
#endif
#ifndef NO_ACTION
		run_init_script("all");
#endif		
	}
	apmib_reinit();
}
#endif //!WLAN_EASY_CONFIG
#endif



void update_wps_configured(int reset_flag)
{
	int is_configured, encrypt1, encrypt2, auth, disabled, iVal, format, shared_type;
	char ssid1[100];
	unsigned char tmpbuf[100];	
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	if(reset_flag == 2)
	{
		wps_config_info.caller_id = CALLED_FROM_CMJSYNCHANDLER;
	}
#endif
	if (wps_config_info.caller_id == CALLED_FROM_WLANHANDLER) {
		apmib_get(MIB_WLAN_SSID, (void *)ssid1);
		apmib_get(MIB_WLAN_MODE, (void *)&iVal);
		if (strcmp(ssid1, (char *)wps_config_info.ssid) || (iVal != wps_config_info.wlan_mode)) {
			apmib_set(MIB_WLAN_WSC_SSID, (void *)ssid1);
			goto configuration_changed;
		}

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_ADVANCEHANDLER) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
		if (encrypt1 == ENCRYPT_WEP && 
			shared_type != wps_config_info.shared_type) {
			if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH) {
				if (wps_config_info.shared_type == AUTH_SHARED) {
					auth = WSC_AUTH_OPEN;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
			else {
				if (wps_config_info.shared_type == AUTH_OPEN ||
					wps_config_info.shared_type == AUTH_BOTH) {
					auth = WSC_AUTH_SHARED;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
		}

		return;
	}

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_DISABLED) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == ENCRYPT_WEP) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH)
			auth = WSC_AUTH_OPEN;
		else
			auth = WSC_AUTH_SHARED;
		encrypt2 = WSC_ENCRYPT_WEP;		
	}
	else if (encrypt1 == ENCRYPT_WPA) {
		auth = WSC_AUTH_WPAPSK;
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else if (encrypt1 == ENCRYPT_WPA2) {
		auth = WSC_AUTH_WPA2PSK;
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else {
		auth = WSC_AUTH_WPA2PSKMIXED;
		encrypt2 = WSC_ENCRYPT_TKIPAES;			

// When mixed mode, if no WPA2-AES, try to use WPA-AES or WPA2-TKIP
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&iVal);
		if (!(iVal &	WPA_CIPHER_AES)) {
			if (encrypt1 &	WPA_CIPHER_AES) {			
				//auth = WSC_AUTH_WPAPSK;
				encrypt2 = WSC_ENCRYPT_AES;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
			else{
				encrypt2 = WSC_ENCRYPT_TKIP;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
		}
//-------------------------------------------- david+2008-01-03
		if(encrypt1==WPA_CIPHER_AES && iVal ==WPA_CIPHER_AES){
			encrypt2 = WSC_ENCRYPT_AES;	
			printf("%s %d\n",__FUNCTION__,__LINE__);			
		}
		//printf("%s %d :auth=0x%02X\n",__FUNCTION__,__LINE__ ,auth);		
		// for correct wsc_auth wsc_encry value when security is mixed mode
	}
	apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
	apmib_set(MIB_WLAN_WSC_ENC, (void *)&encrypt2);

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_WPA || encrypt1 == ENCRYPT_WPA2 || encrypt1 == ENCRYPT_WPA2_MIXED) {
		apmib_get(MIB_WLAN_WPA_AUTH, (void *)&format);
		if (format & 2) { // PSK
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
			apmib_set(MIB_WLAN_WSC_PSK, (void *)tmpbuf);					
		}		
	}
	if (reset_flag) {
		reset_flag = 0;
		apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);		
	}	

	if (wps_config_info.caller_id == CALLED_FROM_WEPHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP, (void *)&encrypt2);
		if (wps_config_info.wep_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&iVal);
		if (wps_config_info.KeyId != iVal)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key3, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key4, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key3,(char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key4, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_WPAHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (wps_config_info.wpa_enc != encrypt1)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt2);
		if (wps_config_info.wpa2_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wpaPSK, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}	
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	else if (wps_config_info.caller_id == CALLED_FROM_CMJSYNCHANDLER) {
		goto configuration_changed;
	}
#endif
	else
		return;
	
configuration_changed:	
	reset_flag = 0;
	apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&disabled);	
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
	//if (!is_configured && !disabled) { //We do not care wsc is enable for disable--20081223
	if (!is_configured) {
		is_configured = 1;
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		if(apmib_get_wlanidx()==0){
			apmib_set_wlanidx(1);			
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			apmib_set_wlanidx(0);			
		}else if(apmib_get_wlanidx()==1){
			apmib_set_wlanidx(0);			
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			apmib_set_wlanidx(1);			
		}
#endif
	}
}

#if 0
static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}
#endif

////////////////////////////////////////////////////////////////////////////////
void apmib_reset_wlan_to_default(unsigned char *wlanif_name)
{
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	SetWlan_idx(wlanif_name);
	memcpy(&pMib->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()], &pMibDef->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()], sizeof(CONFIG_WLAN_SETTING_T));	
	if(strstr((char *)wlanif_name,"vxd") != 0)
	{
		if(apmib_get_wlanidx() == 0)
		{
			sprintf((char *)pMib->repeaterSSID1, (char *)pMib->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()].ssid);
			pMib->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()].wlanDisabled = !pMib->repeaterEnabled1;			
		}
		else
		{
			sprintf((char *)pMib->repeaterSSID2, (char *)pMib->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()].ssid);
			pMib->wlan[apmib_get_wlanidx()][apmib_get_vwlanidx()].wlanDisabled = !pMib->repeaterEnabled2;			
		}
	}
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
}

void updateVapWscDisable(int wlan_root,int value)
{
	int i=0;
	int wlanif_idx = 0;
	char ifname[20]={0};
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	for(i=0;i<(NUM_VWLAN_INTERFACE-1);i++) // vap0~vap3
	{
		memset(ifname,0x00,sizeof(ifname));
		sprintf(ifname,"wlan%d-va%d",wlan_root,i);
		SetWlan_idx(ifname);
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&value);
	}
	memset(ifname,0x00,sizeof(ifname));
	sprintf(ifname,"wlan%d-vxd0",wlan_root);
	SetWlan_idx(ifname);
	apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&value);
	
	memset(ifname,0x00,sizeof(ifname));
	sprintf(ifname,"wlan%d",wlan_root);
	SetWlan_idx(ifname);
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
}

void formWsc(char *postData, int length)
{
	char *strVal, *submitUrl, *strResetUnCfg, *wlanIf;
	char *targetAPSsid, *targetAPMac; /* WPS2DOTX */
	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	char  *unlockclicked;
	int intVal;
	char tmpbuf[200];
	char urlbuf[60];
	int mode;
	int reset_to_unconfig_state_flag = 0;
	int intf_idx;
	// 1104
	int tmpint;	
	char ifname[30];
	extern int wscd_reinit_done;
	int wait_cnt;

	// WPS2DOTX ; 2011-0428
    int idx,idx2;
	char pincodestr_b[20];	
	// WPS2DOTX ; 2011-0428
#ifdef HAVE_WPS	
	extern cyg_flag_t wsc_flag;
#endif

	int bak_wlan_idx = apmib_get_wlanidx();
	int restart_wps = 0;
	
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	int wlanDisabled[2];
	int wlanMode[2];		
	int wlanPhyBand[2];
	int wlanMacPhy[2];
	int wlanif;
	int isSwapWlwanIf = 0;
	int wlan0_mode=1, wlan1_mode=1;
	int wlan0_disable=0, wlan1_disable=0;
	int wlan_orig;
	int wlan0_wsc_disable=0, wlan1_wsc_disable=0;
	int both_band_ap=0, run_on_ap_if=0;

#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	SetWlan_idx("wlan0");
	apmib_get(MIB_WLAN_MODE, &wlan0_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan0_disable);
	apmib_get(MIB_WLAN_WSC_DISABLE, &wlan0_wsc_disable);
	SetWlan_idx("wlan1");
	apmib_get(MIB_WLAN_MODE, &wlan1_mode);
	apmib_get(MIB_WLAN_WLAN_DISABLED, &wlan1_disable);
	apmib_get(MIB_WLAN_WSC_DISABLE, &wlan1_wsc_disable);
	
	sprintf(tmpbuf, "wlan%d", apmib_get_wlanidx());
	SetWlan_idx(tmpbuf);
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif

	if( ((wlan0_mode == AP_MODE) || (wlan0_mode == AP_WDS_MODE)) && ((wlan1_mode == AP_MODE) || (wlan1_mode == AP_WDS_MODE))
			&& (wlan0_wsc_disable == 0) && (wlan1_wsc_disable == 0) && (wlan0_disable == 0) && (wlan1_disable == 0))
		both_band_ap = 1;
	if((wlan0_wsc_disable == 0) && (wlan1_wsc_disable == 0) && (wlan0_disable == 0) && (wlan1_disable == 0))
	{
		if(((wlan0_mode == AP_MODE || wlan0_mode == AP_WDS_MODE) && wlan1_mode == CLIENT_MODE)
			|| ((wlan1_mode == AP_MODE || wlan1_mode == AP_WDS_MODE) && wlan0_mode == CLIENT_MODE) )
			run_on_ap_if = 1;
	}
#endif

	apmib_get(MIB_WSC_INTF_INDEX,&intf_idx);
	memset(ifname,'\0',30);
	sprintf(ifname,"wlan%d",apmib_get_wlanidx());
//displayPostDate(wp->post_data);	
	
	/* support  special MAC , 2011-0505 WPS2DOTX */
	
	targetAPMac = get_cstream_var(postData,length, "targetAPMac", (""));
	targetAPSsid = get_cstream_var(postData,length,  "targetAPSsid", (""));
	/* support  special SSID , 2011-0505 WPS2DOTX */	
	submitUrl = get_cstream_var(postData,length,  ("submit-url"), "");

	strResetUnCfg = get_cstream_var(postData,length,  ("resetUnCfg"), "");
	if(strResetUnCfg[0] && strResetUnCfg[0]=='1')// reset to unconfig state. Keith
	{				
//#if defined(FOR_DUAL_BAND) //  ; both reset two unterface (wlan0 AND wlan1)
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
#if defined(CONFIG_RTL_92D_SUPPORT)
		wlanif = whichWlanIfIs(PHYBAND_5G);
		
		if(wlanif != 0)
		{
			swapWlanMibSetting(0,1);
			isSwapWlwanIf = 1;
		}
#endif
		wlanDisabled[0] = pMib->wlan[0][0].wlanDisabled;
		wlanDisabled[1] = pMib->wlan[1][0].wlanDisabled;
		wlanMode[0] = pMib->wlan[0][0].wlanMode;
		wlanMode[1] = pMib->wlan[1][0].wlanMode;
		wlanMacPhy[0] = pMib->wlan[0][0].macPhyMode;
		wlanMacPhy[1] = pMib->wlan[1][0].macPhyMode;
			
		printf("(%s,%d)Reset to OOB ...\n",__FUNCTION__ , __LINE__);
		if(wlanMode[0] != CLIENT_MODE)
		{
			apmib_reset_wlan_to_default("wlan0");
			pMib->wlan[0][0].wlanDisabled = wlanDisabled[0];
			pMib->wlan[0][0].macPhyMode = wlanMacPhy[0];
		}
		if(wlanMode[1] != CLIENT_MODE)
		{
			apmib_reset_wlan_to_default("wlan1");
			pMib->wlan[1][0].wlanDisabled = wlanDisabled[1];
			pMib->wlan[1][0].macPhyMode = wlanMacPhy[1];
		}
#if defined(CONFIG_RTL_92D_SUPPORT)		
		if(isSwapWlwanIf == 1)
		{
			swapWlanMibSetting(0,1);
			isSwapWlwanIf = 0;
		}
#endif		
#else
//		wlanIf = req_get_cstream_var(wp, ("wlanIf"), "");
//		if(wlanIf[0])
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		intVal = get_gpio_2g5g();
		if(intVal == 2)
			apmib_reset_wlan_to_default((unsigned char *)"wlan1");
		else if(intVal == 5)
			apmib_reset_wlan_to_default((unsigned char *)"wlan0");
#else
		apmib_reset_wlan_to_default((unsigned char *)"wlan0");
#endif
//		else
//			printf("Reset wlan to default fail!! No wlan name. %s,%d\n",__FUNCTION__ , __LINE__);
#endif
#ifdef REBOOT_CHECK		
		strVal = get_cstream_var(postData,length, ("disableWPS"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if(intVal == 1)
		{
			if(bak_wlan_idx == 0)
				tmpint = 2;
			else if(bak_wlan_idx == 1)
				tmpint = 0;
		}
		else if(intVal == 0)
		{
			if(bak_wlan_idx == 0)
			{
				apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)intVal, 1, 0);
				if(intVal == 1)
					tmpint = 0;
				else
					tmpint = 5;
			}
			else if(bak_wlan_idx == 1)
			{
				apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)intVal, 0, 0);
				if(intVal == 1)
					tmpint = 2;
				else
					tmpint = 5;
			}
		}
		apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
		updateVapWscDisable(wlan_idx,intVal);
#endif
		apmib_update_web(CURRENT_SETTING);
#ifdef HAVE_SYSTEM_REINIT
		{
			sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,bak_wlan_idx);
			wait_redirect("Apply Changes", 5,urlbuf);
			sleep(1);
			kick_reinit_m(SYS_WIFI_M|SYS_WIFI_APP_M);
		}
#elif defined(WIFI_QUICK_REINIT)
		kick_event(WLAN_BRIDGE_EVENT);
		send_redirect_perm(submitUrl);
#else
		OK_MSG(submitUrl);
#endif

		save_cs_to_file();

		return;
	}
	
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strVal=get_cstream_var(postData,length, ("config_runon"), "");
	if(strVal[0])
	{
		if(bak_wlan_idx==0)
			intVal=strVal[0]-'0';
		else
			intVal=strVal[0]-'0' + 2; //wlan1 index is 2, wlan1-vxd is 3;
			
		//diag_printf("[%s:%d]intf_idx %d intVal %d\n",__FUNCTION__,__LINE__,intf_idx,intVal);
#if defined(CONFIG_RTL_92D_SUPPORT)|| defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		if(intf_idx != intVal && run_on_ap_if == 0)
#else
		if(intf_idx != intVal)
#endif
		{
			apmib_set(MIB_WSC_INTF_INDEX,(void *)&intVal);	
			restart_wps = 1;
			/*first stop wscd*/
			cyg_flag_setbits(&wsc_flag, 0x80);
			sleep(1);
			/*restart wscd*/			
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
			intf_idx = intVal;
		}	
	}
	
	apmib_get(MIB_WLAN_MODE, (void *)&mode);	

	strResetUnCfg = get_cstream_var(postData,length, ("resetRptUnCfg"), "");
	if(strResetUnCfg[0] && strResetUnCfg[0]=='1')// reset to unconfig state. Keith
	{
		wlanIf = get_cstream_var(postData,length, ("wlanIf"), "");
		if(wlanIf[0])
			apmib_reset_wlan_to_default((unsigned char *)wlanIf);
		else
			printf("Reset wlan to default fail!! No wlan name. %s,%d\n",__FUNCTION__ , __LINE__);		
#ifdef REBOOT_CHECK
		strVal = get_cstream_var(postData,length, ("disableWPS"), "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if(intVal == 1)
		{
			if(bak_wlan_idx == 0)
				tmpint = 2;
			else if(bak_wlan_idx == 1)
				tmpint = 0;
		}
		else if(intVal == 0)
		{
			if(bak_wlan_idx == 0)
			{
				apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)&intVal, 1, 0);
				if(intVal == 1)
					tmpint = 0;
				else
					tmpint = 5;
			}
			else if(bak_wlan_idx == 1)
			{
				apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)&intVal, 0, 0);
				if(intVal == 1)
					tmpint = 2;
				else
					tmpint = 5;
			}
		}
		apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
		updateVapWscDisable(wlan_idx, intVal);
		REBOOT_WAIT(submitUrl);
		run_init_script_flag = 1;
#endif		

		apmib_update_web(CURRENT_SETTING);
		
#ifndef NO_ACTION
		run_init_script("bridge");
#endif

		save_cs_to_file();

		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)


	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	unlockclicked = get_cstream_var(postData,length, ("unlockautolockdown"), "");
	if(unlockclicked[0])
	{
#ifdef HAVE_WPS
		extern void wsc_unlock(void);
		//printf("call wsc_unlock\n");			
		wsc_unlock();
#endif
		OK_MSG1(UNLOCK_MSG, submitUrl);

		save_cs_to_file();

		return;

	}
	/*WPS2DOTX ;check if unlock button be clicked ; brute force attack */
	apmib_get(MIB_WLAN_MODE, (void *)&mode);	
	strVal = get_cstream_var(postData,length, ("triggerPBC"), "");
	if (strVal[0]) {
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		if(both_band_ap == 1)
		{
			if(intf_idx != 5)
			{
#ifdef HAVE_WPS	
				tmpint = 5;
				forceWscStop();
				apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
				cyg_flag_setbits(&wsc_flag, 0x80);
				restart_wps = 1;
				sleep(1);
				wscd_reinit_done = 0;
				cyg_flag_setbits(&wsc_flag, 0x20);
				/*wait for wscd restart*/
				wait_cnt = 0;
				while(!wscd_reinit_done)
				{
					sleep(1);
					if(++wait_cnt>10)
						break;
				}
#endif
			}
		}
		else		
#endif
		if((restart_wps == 0) && (intf_idx != bak_wlan_idx*2))
		{
			/*root runs*/
			tmpint=bak_wlan_idx*2;	
#ifdef HAVE_WPS	
			forceWscStop();

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface
				if(mode == CLIENT_MODE)
					tmpint += 8;
			}
#endif
			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			restart_wps = 1;
			sleep(1);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for wscd restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
#endif
		}
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			intVal = 0;
	
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
                        updateVapWscDisable(apmib_get_wlanidx(), intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			system("echo 1 > /var/wps_start_pbc");
#ifndef NO_ACTION
			run_init_script("bridge");
#endif			
		}
		else {
#ifdef HAVE_WPS
			sleep(1);
			cyg_flag_setbits(&wsc_flag, 8);
/*
#ifndef NO_ACTION
			sprintf(tmpbuf, "%s -sig_pbc wlan%d", _WSC_DAEMON_PROG,apmib_get_wlanidx());
			system(tmpbuf);

#endif
*/
#endif
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);

		save_cs_to_file();

		return;
	}
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)	
	strVal = get_cstream_var(postData,length, ("triggerRptPBC"), "");
	if (strVal[0]) {
		if((restart_wps == 0) && (intf_idx != bak_wlan_idx*2+1))
		{
			tmpint=bak_wlan_idx*2+1;
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface
				if(mode == AP_MODE)
					tmpint += 8;
			}
#endif
			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			restart_wps = 1;
			sleep(2);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
		}
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			intVal = 0;
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			updateVapWscDisable(apmib_get_wlanidx(), intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
			sleep(2);
			cyg_flag_setbits(&wsc_flag, 8);
#ifndef NO_ACTION
			run_init_script("bridge");
#endif			
		}
		else {	
			sleep(2);
#ifdef HAVE_WPS	
			cyg_flag_setbits(&wsc_flag, 8);
/*
#ifndef NO_ACTION	
			sprintf(tmpbuf, "%s -sig_pbc wlan%d-vxd", _WSC_DAEMON_PROG,apmib_get_wlanidx());
			system(tmpbuf);
#endif
*/
#endif
		}
		OK_MSG2(START_PBC_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);

		save_cs_to_file();

		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)

/* support  special SSID , 2011-0505 WPS2DOTX */
	strVal = get_cstream_var(postData,length, ("stopwsc"), (""));
	if (strVal[0]) {
		//system("echo 1 > /tmp/wscd_cancel");	
#ifdef HAVE_WPS	
		extern void set_wps_interface();
		set_wps_interface();
		cyg_flag_setbits(&wsc_flag, 0x80);
		sleep(1);
		wscd_reinit_done = 0;
		cyg_flag_setbits(&wsc_flag, 0x20);/*restart wsc, later wsc will always run on AP interface*/
		/*wait for wscd restart*/
		wait_cnt = 0;
		while(!wscd_reinit_done)
		{
			sleep(1);
			if(++wait_cnt>10)
				break;
		}
#endif
		OK_MSG2(STOP_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);

		save_cs_to_file();

		return;
	}
/* support  special SSID , 2011-0505 WPS2DOTX */

	strVal = get_cstream_var(postData,length, ("triggerPIN"), "");
	if (strVal[0]) {
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		if(both_band_ap == 1)
		{
			if(intf_idx != 5)
			{
#ifdef HAVE_WPS	
				tmpint = 5;
				forceWscStop();
				apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
				cyg_flag_setbits(&wsc_flag, 0x80);
				restart_wps = 1;
				sleep(1);
				wscd_reinit_done = 0;
				cyg_flag_setbits(&wsc_flag, 0x20);
				/*wait for wscd restart*/
				wait_cnt = 0;
				while(!wscd_reinit_done)
				{
					sleep(1);
					if(++wait_cnt>10)
						break;
				}
#endif
			}
		}
		else		
#endif		
		if(bak_wlan_idx*2  != intf_idx)
		{
			/*root runs*/
			tmpint=bak_wlan_idx*2;
#ifdef HAVE_WPS	
			forceWscStop();

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface
				if(mode == CLIENT_MODE)
					tmpint += 8;
			}
#endif
			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			sleep(1);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for wscd restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
#endif
		}
		int local_pin_changed = 0;		
		strVal = get_cstream_var(postData,length,  ("localPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_HW_WSC_PIN, (void *)tmpbuf);
			if (strcmp(tmpbuf, strVal)) {
				apmib_set(MIB_HW_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;				
			}			
		}		
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			char localpin[100];
			intVal = 0;			
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			updateVapWscDisable(apmib_get_wlanidx(), intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash	
#ifdef HAVE_WPS	
			cyg_flag_setbits(&wsc_flag, 4);
#endif
			//system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				apmib_get(MIB_HW_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");			
#endif			
		}
		else {	
			
#ifndef NO_ACTION		
			if (local_pin_changed) {
#ifdef HAVE_WPS					
				cyg_flag_setbits(&wsc_flag, 4);
#endif
				//system("echo 1 > /var/wps_start_pin");
				
				apmib_update_web(CURRENT_SETTING);					
				run_init_script("bridge");
			}
			else {

				/* support  special MAC , 2011-0505 ;WPS2DOTX*/
				if(targetAPMac[0]){
					unsigned char targetAPMacFilter[20];
					int idx = 0;
					int idx2 = 0;					
					//printf("before ,mac =%s len=%d \n",targetAPMac , strlen(targetAPMac));
					for(idx;idx<strlen(targetAPMac);idx++){
						if( _is_hex(targetAPMac[idx])){
							targetAPMacFilter[idx2]=targetAPMac[idx];
							idx2++;
						}
					}
					
					targetAPMacFilter[idx2]='\0';
					
					if(strlen(targetAPMacFilter)!=12){
						printf("invaild MAC Addr Len\n\n");
					}else{
						/*
						sprintf(tmpbuf, "iwpriv wlan%d set_mib wsc_specmac=%s ",apmib_get_wlanidx(), targetAPMacFilter);
						system(tmpbuf);	
						*/
						sprintf(tmpbuf, "wsc_specmac=%s", targetAPMacFilter);
						sprintf(ifname, "wlan%d", bak_wlan_idx);
						RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", tmpbuf, NULL_STR);
					}											
					
				}
				if(targetAPSsid[0]){					
					if(strlen(targetAPSsid)<= 32){
						/*
						sprintf(tmpbuf, "iwpriv wlan%d set_mib wsc_specssid=\"%s\" ",apmib_get_wlanidx(), targetAPSsid);
						system(tmpbuf);						
						*/
						sprintf(tmpbuf, "wsc_specssid=%s", targetAPSsid);
						sprintf(ifname, "wlan%d", bak_wlan_idx);
						RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", tmpbuf, NULL_STR);
					}else{					
						printf("invaild SSID Len\n");
					}											
					
				}				
				/* support  special SSID , 2011-0505 WPS2DOTX */
#if 0//defined(FOR_DUAL_BAND)
				if( (wlan0_mode == 0) && (wlan1_mode == 0) && (wlan0_disable == 0) && (wlan1_disable == 0))
						sprintf(tmpbuf, "%s -sig_start %s", _WSC_DAEMON_PROG, "wlan0-wlan1");
				else 
						sprintf(tmpbuf, "%s -sig_start wlan%d", _WSC_DAEMON_PROG,apmib_get_wlanidx());
				system(tmpbuf);
#else
#ifdef HAVE_WPS	
				cyg_flag_setbits(&wsc_flag, 4);
#endif
				/*
				sprintf(tmpbuf, "%s -sig_start wlan%d", _WSC_DAEMON_PROG,apmib_get_wlanidx());
				system(tmpbuf);
				*/
#endif
			}			
#endif
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);

		save_cs_to_file();

		return;
	}
	
#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strVal = get_cstream_var(postData,length,  ("triggerRptPIN"), "");

	if (strVal[0]) {
		int local_pin_changed = 0;		
		if((restart_wps == 0) && (bak_wlan_idx*2+1 != intf_idx))
		{
			tmpint=bak_wlan_idx*2+1;
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface, 
				//the root interface is AP mode, vxd is client mode
				if(mode == AP_MODE)
					tmpint += 8;
			}
#endif
			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			sleep(1);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
		}
		strVal = get_cstream_var(postData,length,  ("localPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_HW_WSC_PIN, (void *)tmpbuf);

			if (strcmp(tmpbuf, strVal)) {
				apmib_set(MIB_HW_WSC_PIN, (void *)strVal);
				local_pin_changed = 1;				
			}			
		}		
		apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
		if (intVal) {
			char localpin[100];
			intVal = 0;			
			apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			apmib_update_web(CURRENT_SETTING);	// update to flash
#ifdef HAVE_WPS	
			cyg_flag_setbits(&wsc_flag, 4);
#endif
			//system("echo 1 > /var/wps_start_pin");

#ifndef NO_ACTION
			if (local_pin_changed) {
				apmib_get(MIB_HW_WSC_PIN, (void *)localpin);
				sprintf(tmpbuf, "echo %s > /var/wps_local_pin", localpin);
				system(tmpbuf);
			}
			run_init_script("bridge");			
#endif			
		}
		else {		
#ifndef NO_ACTION		
			if (local_pin_changed) {
#ifdef HAVE_WPS	
				cyg_flag_setbits(&wsc_flag, 4);
#endif
				//system("echo 1 > /var/wps_start_pin");
				
				apmib_update_web(CURRENT_SETTING);					
				run_init_script("bridge");
			}
			else {
#ifdef HAVE_WPS	
				cyg_flag_setbits(&wsc_flag, 4);
#endif
/*
				sprintf(tmpbuf, "%s -sig_start wlan%d-vxd", _WSC_DAEMON_PROG,wlan_idx);
				system(tmpbuf);
				*/
			}			
#endif
		}
		OK_MSG2(START_PIN_MSG, ((mode==AP_MODE) ? "client" : "AP"), submitUrl);

		save_cs_to_file();

		return;
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)		
	
	strVal = get_cstream_var(postData,length,  ("setPIN"), "");
	if (strVal[0]) {	
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		if(both_band_ap == 1)
		{
			if(intf_idx != 5)
			{
#ifdef HAVE_WPS	
				tmpint = 5;
				forceWscStop();
				apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
				cyg_flag_setbits(&wsc_flag, 0x80);
				restart_wps = 1;
				sleep(1);
				wscd_reinit_done = 0;
				cyg_flag_setbits(&wsc_flag, 0x20);
				/*wait for wscd restart*/
				wait_cnt = 0;
				while(!wscd_reinit_done)
				{
					sleep(1);
					if(++wait_cnt>10)
						break;
				}
#endif
			}
		}
		else		
#endif		
		if(bak_wlan_idx*2 != intf_idx)
		{
			/*root runs*/
			tmpint=bak_wlan_idx*2;
#ifdef HAVE_WPS	
			forceWscStop();
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface, 
				if(mode == CLIENT_MODE)
					tmpint += 8;
			}
#endif

			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			sleep(1);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for wscd restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
#endif
		}
		strVal = get_cstream_var(postData,length, ("peerPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			if (intVal) {
				intVal = 0;
				apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
				apmib_update_web(CURRENT_SETTING);	
#ifdef HAVE_WPS	
				cyg_flag_setbits(&wsc_flag, 4);
#endif

				//sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				//system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif					
			}
			else {			
#ifndef NO_ACTION
				// WPS2DOTX ; 2011-0428 ; support the format pin code 1234-5670 (include "-")
				memset(pincodestr_b,'\0',20);				
				//printf("before filter pin code =%s , len =%d\n", strVal ,strlen(strVal));
				idx2=0;
				for(idx=0 ; idx <strlen(strVal) ; idx++){
					//printf("strVal[%d]=%x\n",idx,strVal[idx]);	
					if(strVal[idx] >= '0' && strVal[idx]<= '9'){
						pincodestr_b[idx2]=strVal[idx];	
						idx2++;
					}
				}
				
				//printf("after filter pin code =%s , len =%d\n", pincodestr_b ,strlen(pincodestr_b));				
				//sprintf(tmpbuf, "iwpriv %s set_mib pin=%s", ifname, pincodestr_b);				
				//printf("tmpbuf=%s\n",tmpbuf);
				//system(tmpbuf);
				sprintf(tmpbuf, "pin=%s", pincodestr_b);
				RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", tmpbuf, NULL_STR);
				// WPS2DOTX ; 2011-0428 ; support the format pin code 1234-5670 (include "-")				
#endif
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);

			save_cs_to_file();

			return;
		}
	}

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	strVal = get_cstream_var(postData,length, ("setRptPIN"), "");
	if (strVal[0]) {	
		if((bak_wlan_idx*2+1) != intf_idx)
		{
			tmpint=bak_wlan_idx*2+1;
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if(run_on_ap_if == 1)
			{
				apmib_get(MIB_WLAN_MODE, &mode);
				//after WPS, the wscd daemon will run on AP mode interface, 
				//the root interface is AP mode, vxd is client mode
				if(mode == AP_MODE)
					tmpint += 8;
			}
#endif
			apmib_set(MIB_WSC_INTF_INDEX,&tmpint);
			cyg_flag_setbits(&wsc_flag, 0x80);
			sleep(1);
			wscd_reinit_done = 0;
			cyg_flag_setbits(&wsc_flag, 0x20);
			/*wait for restart*/
			wait_cnt = 0;
			while(!wscd_reinit_done)
			{
				sleep(1);
				if(++wait_cnt>10)
					break;
			}
		}
		strVal = get_cstream_var(postData,length,  ("peerRptPin"), "");
		if (strVal[0]) {
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
			if (intVal) {
				intVal = 0;
				apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);
				apmib_update_web(CURRENT_SETTING);	

				sprintf(tmpbuf, "echo %s > /var/wps_peer_pin", strVal);
				system(tmpbuf);

#ifndef NO_ACTION
				run_init_script("bridge");
#endif					
			}
			else {			
#ifndef NO_ACTION
				//sprintf(tmpbuf, "iwpriv wlan%d-vxd set_mib pin=%s", wlan_idx, strVal);
				//system(tmpbuf);
				sprintf(tmpbuf, "iwpriv wlan%d-vxd0 set_mib pin=%s", apmib_get_wlanidx(), strVal);
				RunSystemCmd(NULL_FILE, tmpbuf, NULL_STR);
#endif
			}
			OK_MSG1(SET_PIN_MSG, submitUrl);		

			save_cs_to_file();

			return;
		}
	}
#endif //#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)

	strVal = get_cstream_var(postData,length,  ("disableWPS"), "");
	if ( !strcmp(strVal, "ON"))
		intVal = 1;
	else
		intVal = 0;

	// 1104
	SetWlan_idx(ifname);
	apmib_set(MIB_WLAN_WSC_DISABLE, (void *)&intVal);

	if(intVal == 1)
	{
		if(bak_wlan_idx == 0)
			tmpint = 2;
		else if(bak_wlan_idx == 1)
			tmpint = 0;
	}
	else if(intVal == 0)
	{
		if(bak_wlan_idx == 0)
		{
			apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)&intVal, 1, 0);
			if(intVal == 1)
				tmpint = 0;
			else
				tmpint = 5;
		}
		else if(bak_wlan_idx == 1)
		{
			apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)&intVal, 0, 0);
			if(intVal == 1)
				tmpint = 2;
			else
				tmpint = 5;
		}
	}
	apmib_set(MIB_WSC_INTF_INDEX, (void *)&tmpint);

	
	updateVapWscDisable(apmib_get_wlanidx(), intVal);

	strVal = get_cstream_var(postData,length,  ("localPin"), "");
	if (strVal[0])
		apmib_set(MIB_HW_WSC_PIN, (void *)strVal);

//	update_wps_configured(0);
		
	apmib_update_web(CURRENT_SETTING);	// update to flash
	
#ifndef NO_ACTION
//	run_init_script("bridge");
#endif
#ifdef HAVE_SYSTEM_REINIT
	{
		sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,bak_wlan_idx);
		wait_redirect("Apply Changes", 5,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_APP_M);
	}
#else
	OK_MSG(submitUrl);
#endif

	save_cs_to_file();
}
////////////////////////////////////////////////////////////////////////
#endif // WIFI_SIMPLE_CONFIG

#ifdef UNIVERSAL_REPEATER
void setRepeaterSsid(int wlanid, int rptid, char *str_ssid)
{
	char wlanifStr[10];
	char tmpStr[MAX_SSID_LEN];		
	
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	sprintf(wlanifStr,"wlan%d-vxd0",wlanid);
	SetWlan_idx(wlanifStr);	
	
	apmib_get(MIB_WLAN_SSID, (void *)tmpStr);
	
	if(strcmp(tmpStr, str_ssid) != 0 && strcmp(str_ssid, tmpStr) != 0)
	{
		int is_configured = 1;
		
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
		
		sprintf(tmpStr,"%s",str_ssid);
		apmib_set(MIB_WLAN_SSID, (void *)tmpStr);
		apmib_set(MIB_WLAN_WSC_SSID, (void *)tmpStr);	
		apmib_set(rptid, (void *)tmpStr);
	}
	
	sprintf(wlanifStr,"wlan%d",wlanid);
	SetWlan_idx(wlanifStr);
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
}
#endif


#if 0
static void update_wps_configured(int reset_flag)
{
	int is_configured, encrypt1, encrypt2, auth, disabled, iVal, format, shared_type;
	char ssid1[100];
	unsigned char tmpbuf[100];	
	
	if (wps_config_info.caller_id == CALLED_FROM_WLANHANDLER) {
		apmib_get(MIB_WLAN_SSID, (void *)ssid1);
		apmib_get(MIB_WLAN_MODE, (void *)&iVal);
		if (strcmp(ssid1, (char *)wps_config_info.ssid) || (iVal != wps_config_info.wlan_mode)) {
			apmib_set(MIB_WLAN_WSC_SSID, (void *)ssid1);
			goto configuration_changed;
		}

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_ADVANCEHANDLER) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
		if (encrypt1 == ENCRYPT_WEP && 
			shared_type != wps_config_info.shared_type) {
			if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH) {
				if (wps_config_info.shared_type == AUTH_SHARED) {
					auth = WSC_AUTH_OPEN;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
			else {
				if (wps_config_info.shared_type == AUTH_OPEN ||
					wps_config_info.shared_type == AUTH_BOTH) {
					auth = WSC_AUTH_SHARED;
					apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
					goto configuration_changed;
				}
			}
		}

		return;
	}

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_DISABLED) {
		auth = WSC_AUTH_OPEN;
		encrypt2 = WSC_ENCRYPT_NONE;
	}
	else if (encrypt1 == ENCRYPT_WEP) {
		apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&shared_type);
		if (shared_type == AUTH_OPEN || shared_type == AUTH_BOTH)
			auth = WSC_AUTH_OPEN;
		else
			auth = WSC_AUTH_SHARED;
		encrypt2 = WSC_ENCRYPT_WEP;		
	}
	else if (encrypt1 == ENCRYPT_WPA) {
		auth = WSC_AUTH_WPAPSK;
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else if (encrypt1 == ENCRYPT_WPA2) {
		auth = WSC_AUTH_WPA2PSK;
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt1);
		if (encrypt1 == WPA_CIPHER_TKIP)
			encrypt2 = WSC_ENCRYPT_TKIP;		
		else if (encrypt1 == WPA_CIPHER_AES)
			encrypt2 = WSC_ENCRYPT_AES;		
		else 
			encrypt2 = WSC_ENCRYPT_TKIPAES;				
	}
	else {
		auth = WSC_AUTH_WPA2PSKMIXED;
		encrypt2 = WSC_ENCRYPT_TKIPAES;			

// When mixed mode, if no WPA2-AES, try to use WPA-AES or WPA2-TKIP
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&iVal);
		if (!(iVal &	WPA_CIPHER_AES)) {
			if (encrypt1 &	WPA_CIPHER_AES) {			
				//auth = WSC_AUTH_WPAPSK;
				encrypt2 = WSC_ENCRYPT_AES;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
			else{
				encrypt2 = WSC_ENCRYPT_TKIP;	
				//printf("%s %d : %d\n",__FUNCTION__,__LINE__ ,encrypt2);
			}
		}
//-------------------------------------------- david+2008-01-03
		if(encrypt1==WPA_CIPHER_AES && iVal ==WPA_CIPHER_AES){
			encrypt2 = WSC_ENCRYPT_AES;	
			printf("%s %d\n",__FUNCTION__,__LINE__);			
		}
		//printf("%s %d :auth=0x%02X\n",__FUNCTION__,__LINE__ ,auth);		
		// for correct wsc_auth wsc_encry value when security is mixed mode
	}
	apmib_set(MIB_WLAN_WSC_AUTH, (void *)&auth);
	apmib_set(MIB_WLAN_WSC_ENC, (void *)&encrypt2);

	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt1);
	if (encrypt1 == ENCRYPT_WPA || encrypt1 == ENCRYPT_WPA2 || encrypt1 == ENCRYPT_WPA2_MIXED) {
		apmib_get(MIB_WLAN_WPA_AUTH, (void *)&format);
		if (format & 2) { // PSK
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
			apmib_set(MIB_WLAN_WSC_PSK, (void *)tmpbuf);					
		}		
	}
	if (reset_flag) {
		reset_flag = 0;
		apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);		
	}	

	if (wps_config_info.caller_id == CALLED_FROM_WEPHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP, (void *)&encrypt2);
		if (wps_config_info.wep_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&iVal);
		if (wps_config_info.KeyId != iVal)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key3, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep64Key4, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key1, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key2, (char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key3,(char *)tmpbuf))
			goto configuration_changed;

		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wep128Key4, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}
	else if (wps_config_info.caller_id == CALLED_FROM_WPAHANDLER) {
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&auth);
		if (wps_config_info.auth != auth)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&encrypt1);
		if (wps_config_info.wpa_enc != encrypt1)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&encrypt2);
		if (wps_config_info.wpa2_enc != encrypt2)
			goto configuration_changed;
		
		apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpbuf);
		if (strcmp((char *)wps_config_info.wpaPSK, (char *)tmpbuf))
			goto configuration_changed;

		return;
	}
	else
		return;
	
configuration_changed:	
	reset_flag = 0;
	apmib_set(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&reset_flag);
	apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&disabled);	
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
	//if (!is_configured && !disabled) { //We do not care wsc is enable for disable--20081223
	if (!is_configured) {
		is_configured = 1;
		apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
#if defined(CONFIG_RTL_92D_SUPPORT)
		if(wlan_idx==0){
			wlan_idx = 1;
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			wlan_idx = 0;			
		}else if(wlan_idx == 1){
			wlan_idx = 0;
			apmib_set(MIB_WLAN_WSC_CONFIGURED, (void *)&is_configured);
			wlan_idx = 1;			
		}
#endif
	}
}
#endif

#if defined(WLAN_PROFILE)
int addClientWpsProfile()
{
	int profile_enabled_id, profileEnabledVal, wlan_mode;

	apmib_get( MIB_PROFILE_ENABLED1, (void *)&profileEnabledVal);
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if((profileEnabledVal == 1) && (wlan_mode == CLIENT_MODE))
	{
		char tmpBuf[128]="wps_client_profile";
		char *wp=NULL;
		addWlProfileHandler(wp, 32, tmpBuf, 0);
		apmib_update(CURRENT_SETTING);
	}
}
#else
int addClientWpsProfile()
{
	return 0;
}
#endif

#if defined(WLAN_PROFILE)

int addWlProfileHandler(char *postData, int len, char *tmpBuf, int wlan_id)
{
	char *tmpStr;
	char varName[20];

	int add_to_profile = 0;
	
	if(!memcmp(tmpBuf, "wps_client_profile", 18))
	{
		add_to_profile=1;
	}

	if(add_to_profile == 0)
	{
			sprintf(varName, "wizardAddProfile%d", wlan_id);
			tmpStr = get_cstream_var(postData,len, varName, "");
			if(tmpStr[0])
			add_to_profile = 1;
	}
	
	if(add_to_profile == 1)
	{
		int profile_num_id, profile_tbl_id, profile_add_id, profile_delall_id;
		WLAN_PROFILE_T entry;
		char strSSID[64]={0};
		int encryptVal;
		int entryNum;
//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);
		if(wlan_id == 0)
		{
			profile_num_id = MIB_PROFILE_NUM1;
			profile_tbl_id = MIB_PROFILE_TBL1;
			profile_add_id = MIB_PROFILE_ADD1;
			profile_delall_id = MIB_PROFILE_DELALL1;
		}
		else
		{
			profile_num_id = MIB_PROFILE_NUM2;
			profile_tbl_id = MIB_PROFILE_TBL2;
			profile_add_id = MIB_PROFILE_ADD2;
			profile_delall_id = MIB_PROFILE_DELALL2;			
		}
		
		apmib_get(profile_num_id, (void *)&entryNum);

		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_end;
		}
		
		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8;//WPA_CIPHER_AES
		
		

		apmib_get(MIB_WLAN_SSID, (void *)strSSID);
		strcpy(entry.ssid, strSSID);

		apmib_get(MIB_WLAN_ENCRYPT, (void *)&encryptVal);

//printf("\r\n encryptVal=[%d],__[%s-%u]\r\n",encryptVal,__FILE__,__LINE__);	

		if(encryptVal == ENCRYPT_WEP)
		{
			int wepType;
			int authType;
			int defKey;
			char key[30];
			int keyType;
			
			apmib_get( MIB_WLAN_WEP, (void *)&wepType);
			if(wepType == WEP64)
			{
				entry.encryption = WEP64;

				memset(key, 0x00, sizeof(key));					
				apmib_get(MIB_WLAN_WEP64_KEY1, (void *)key);
				strcpy(entry.wepKey1, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY2, (void *)key);
				strcpy(entry.wepKey2, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY3, (void *)key);
				strcpy(entry.wepKey3, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP64_KEY4, (void *)key);
				strcpy(entry.wepKey4, key);
			}
			else
			{
				entry.encryption = WEP128;
				
				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY1, (void *)key);
				strcpy(entry.wepKey1, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY2, (void *)key);
				strcpy(entry.wepKey2, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY3, (void *)key);
				strcpy(entry.wepKey3, key);

				memset(key, 0x00, sizeof(key));
				apmib_get(MIB_WLAN_WEP128_KEY4, (void *)key);
				strcpy(entry.wepKey4, key);
			}

			apmib_get( MIB_WLAN_AUTH_TYPE, (void *)&authType);
			entry.auth = authType;

			apmib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&defKey);
			entry.wep_default_key = defKey;

			apmib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&keyType);
			entry.wepKeyType = keyType;

			
		}
		else if(encryptVal > ENCRYPT_WEP)
		{
			int cipherSuite;
			int pskFormat;
			char wpaPsk[65]={0};
			
			if(encryptVal== ENCRYPT_WPA)
			{
				entry.encryption = 3;
				apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&cipherSuite);
			}
			else
			{
				entry.encryption = 4;
				apmib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&cipherSuite);
			}

			
			if(cipherSuite == WPA_CIPHER_TKIP)
				entry.wpa_cipher = 2;
			else
				entry.wpa_cipher = 8;

			apmib_get( MIB_WLAN_PSK_FORMAT, (void *)&pskFormat);
			entry.wpaPSKFormat = pskFormat;

			apmib_get(MIB_WLAN_WPA_PSK,  (void *)wpaPsk);

			strcpy(entry.wpaPSK, wpaPsk);
			
		}
		else
		{
			entry.encryption = ENCRYPT_DISABLED;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));

//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}

		apmib_set(profile_delall_id, (void *)&entry);

#endif

		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_end;
		}

#if defined(PROFILE_BOTTOM_UP)
		for(roop=0 ; roop<entryNum; roop++)
		{
			if(!memcmp((oriEntry[roop]).ssid, entry.ssid, strlen(entry.ssid)))
			{
				printf("this profile is exist, replace it with new configuration\n");
				continue;
			}
			else
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#endif



		

	}

	return 0 ;
	
setErr_end:
	return -1 ;	
}

static int wlProfileHandler(char *postData, int len, char *tmpBuf, int wlan_id)
{
	char *strVal, *strSSID;
	char varName[20], strtmp[80];
	char *strAddWlProfile, *strAddRptProfile, *strDelSelProfile, *strDelAllProfile, *strWlProfileEnabled;
	int profile_num_id, profile_tbl_id, profile_add_id, profile_del_id, profile_delall_id, profile_enabled_id;
	int entryNum;
	int wlProfileEnabled;
	WLAN_PROFILE_T entry;
	int mode;
	int val;
	
//displayPostDate(wp->post_data);

	sprintf(varName, "mode%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");

	if(strVal[0] == 0)
	{
		int val;

		apmib_get( MIB_WLAN_MODE, (void *)&val);
		sprintf(strtmp,"%d",val);
		strVal = strtmp;		
	}

	
	if ( strVal[0] ) {
		mode = strVal[0] - '0';

		if (mode == CLIENT_MODE) {
			ENCRYPT_T encrypt;
      		apmib_get( MIB_WLAN_ENCRYPT,  (void *)&encrypt);
		if (encrypt &  ENCRYPT_WPA2_MIXED) {
			int format;
			apmib_get( MIB_WLAN_WPA_AUTH, (void *)&format);
			if (format & 1) { // radius
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				//Support wlan client mode with Enterprise (RADIUS)
#else
				strcpy(tmpBuf, ("You cannot set client mode with Enterprise (RADIUS) !<br><br>Please change the encryption method in security page first."));
				goto setErr_wlan;
#endif
			}
		}
		else if (encrypt == ENCRYPT_WEP || encrypt == 0) {
			int use1x;
			apmib_get( MIB_WLAN_ENABLE_1X, (void *)&use1x);
			if (use1x & 1) {
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				//Support wlan client mode with Enterprise (RADIUS)
#else
				strcpy(tmpBuf, ("You cannot set client mode with 802.1x enabled!<br><br>Please change the encryption method in security page first."));
				goto setErr_wlan;
#endif
			}
		}
		sprintf(varName, "wlanMacClone%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if ( !strcmp(strVal, "ON"))
			val = 1 ;
		else
			val = 0 ;
		if ( apmib_set( MIB_WLAN_MACCLONE_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set wlan Mac clone error!"));
			goto setErr_wlan;
		}
	}

	if ( apmib_set( MIB_WLAN_MODE, (void *)&mode) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_MODE error!"));
		goto setErr_wlan;
	}
	
#ifdef WLAN_EASY_CONFIG
	apmib_set( MIB_WLAN_EASYCFG_WLAN_MODE, (void *)&mode);
#endif
	

}
		
	if(wlan_id == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_add_id = MIB_PROFILE_ADD1;
		profile_del_id = MIB_PROFILE_DEL1;
		profile_delall_id = MIB_PROFILE_DELALL1;
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_add_id = MIB_PROFILE_ADD2;		
		profile_del_id = MIB_PROFILE_DEL2;
		profile_delall_id = MIB_PROFILE_DELALL2;
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}
	

	strWlProfileEnabled = get_cstream_var(postData,len, ("wlProfileEnabled"), "");
	strAddWlProfile = get_cstream_var(postData,len, ("addWlProfile"), "");
	strAddRptProfile = get_cstream_var(postData,len, ("addRptProfile"), "");
	strDelSelProfile = get_cstream_var(postData,len, ("delSelWlProfile"), "");
	strDelAllProfile = get_cstream_var(postData,len, ("delAllWlProfile"), "");
	
	if (strWlProfileEnabled[0]) {
			wlProfileEnabled = atoi(strWlProfileEnabled);
			apmib_set( profile_enabled_id, (void *)&wlProfileEnabled);			
	}
	else
	{
		wlProfileEnabled = 0;
		apmib_set( profile_enabled_id, (void *)&wlProfileEnabled);
	}
	
	/* Add entry */
	if (strAddWlProfile[0]) 
	{
		int del_ret; 

		memset(&entry,0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
		sprintf(varName, "ssid%d", wlan_id);
   		strSSID = get_cstream_var(postData,len, varName, "");
   		strcpy(entry.ssid, strSSID);
   	

		apmib_get(profile_num_id, (void *)&entryNum);
		
		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));
			if(!memcmp(&entry,oriEntry+roop,sizeof(entry)))
			{
				strcpy(tmpBuf, ("Add Profile duplicately!"));
				goto setErr_wlan;
			}
//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}		
		apmib_set(profile_delall_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}
		for(roop=0 ; roop<entryNum; roop++)
		{
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#else
		// set to MIB. try to delete it first to avoid duplicate case
		del_ret = apmib_set(profile_del_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}
		if(del_ret == 1)
		{
			//printf("\r\n Duplicate add profile__[%s-%u]\r\n",__FILE__,__LINE__);
			strcpy(tmpBuf, ("Add Profile duplicately!"));
			goto setErr_wlan;
		}
#endif
		sprintf(varName, "ssid%d", wlan_id);
		strSSID = get_cstream_var(postData,len, varName, "");
		if ( strSSID[0] ) {
			if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
	   	 			strcpy(tmpBuf, ("Set SSID error!"));
					goto setErr_wlan;
			}
		}
		else if ( mode == 1 && !strSSID[0] ) { // client and NULL SSID
			if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
	   	 			strcpy(tmpBuf, ("Set SSID error!"));
					goto setErr_wlan;
			}
		}

	}

	if (strAddRptProfile[0]) 
	{
		int id, rpt_enabled;
		int del_ret; 

		memset(&entry,0x00, sizeof(WLAN_PROFILE_T));
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
		sprintf(varName, "repeaterSSID%d", wlan_id);
   	strSSID = get_cstream_var(postData,len, varName, "");
   	strcpy(entry.ssid, strSSID);
   	
		apmib_get(profile_num_id, (void *)&entryNum);
		
		if ( (entryNum + 1) > MAX_WLAN_PROFILE_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wlan;
		}

#if defined(PROFILE_BOTTOM_UP)
		WLAN_PROFILE_T oriEntry[MAX_WLAN_PROFILE_NUM];
		int roop=0;

		for(roop=0 ; roop<entryNum; roop++)
		{
			memset(oriEntry+roop, 0x00, sizeof(WLAN_PROFILE_T));
			*((char *)(oriEntry+roop)) = (char)(roop+1);
			apmib_get(profile_tbl_id, (void *)(oriEntry+roop));
			if(!memcmp(&entry,oriEntry+roop,sizeof(entry)))
			{
				strcpy(tmpBuf, ("Add Profile duplicately!"));
				goto setErr_wlan;
			}
//printf("\r\n oriEntry[roop].ssid=[%s],__[%s-%u]\r\n",oriEntry[roop].ssid,__FILE__,__LINE__);

		}

		apmib_set(profile_delall_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}
		for(roop=0 ; roop<entryNum; roop++)
		{
			apmib_set(profile_add_id, (void *)(oriEntry+roop));
		}
#else
		// set to MIB. try to delete it first to avoid duplicate case
		del_ret = apmib_set(profile_del_id, (void *)&entry);
		if ( apmib_set(profile_add_id, (void *)&entry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wlan;
		}
		if(del_ret == 1)
		{
			//printf("\r\n Duplicate add profile__[%s-%u]\r\n",__FILE__,__LINE__);
			strcpy(tmpBuf, ("Add Profile duplicately!"));
			goto setErr_wlan;
		}
#endif		
		
		sprintf(varName, "repeaterEnabled%d", wlan_id);

		
		strVal = get_cstream_var(postData,len, varName, "");
		if ( !strcmp(strVal, "ON"))
			val = 1 ;
		else
			val = 0 ;
			
#if defined(CONFIG_RTL_ULINKER)
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_get(id, (void *)&rpt_enabled);
		if(mode == AP_MODE && rpt_enabled == 1) //ulinker repeater mode
		{
			val = 1;
		}
#endif
			
		if (wlan_id == 0)
			id = MIB_REPEATER_ENABLED1;
		else
			id = MIB_REPEATER_ENABLED2;
		apmib_set(id, (void *)&val);

		if (val == 1) {
			sprintf(varName, "repeaterSSID%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, NULL);
			if (strVal){
				if (wlan_id == 0)
					id = MIB_REPEATER_SSID1;
				else
					id = MIB_REPEATER_SSID2;
					
				setRepeaterSsid(wlan_id, id, strVal);
			}
		}

	}


	
	/* Delete entry */
	if (strDelSelProfile[0]) {
		int i;
		apmib_get(profile_num_id, (void *)&entryNum);

			
		for (i=entryNum; i>0; i--) {
			strVal = NULL;
			snprintf(strtmp, 20, "select%d", i);

			strVal = get_cstream_var(postData,len, strtmp, "");
			if(strVal == NULL)
				continue;
				
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&entry) = (char)i;
				
				if ( !apmib_get(profile_tbl_id, (void *)&entry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_wlan;
				}
				if ( !apmib_set(profile_del_id, (void *)&entry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_wlan;
				}
			}
		}

	}

	/* Delete all entry */
	if ( strDelAllProfile[0]) {

		if ( !apmib_set(profile_delall_id, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_wlan;
		}
	}


	return  0;

setErr_wlan:
	return -1 ;	
	
}

#endif //#if defined(WLAN_PROFILE)


int wlanHandler(char *postData,int len,char *tmpBuf, int *mode, int wlan_id)
{
  	char *strSSID, *strChan, *strDisabled, *strVal, strtmp[80];
	int chan, disabled ;
	NETWORK_TYPE_T net;
	char *strRate;
	int val;
	char varName[20];
	int band_no=0;
	int cur_band=0;
	int wlanBand2G5GSelect;
#if defined(WIFI_QUICK_REINIT)
	int quick_reinit = 1;
	int old_value = 0;
#endif
	
	apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
	sprintf(varName, "wlanDisabled%d", wlan_id);
	strDisabled = get_cstream_var(postData,len, varName, "");
	if ( !strcmp(strDisabled, "ON"))
		disabled = 1;
	else
		disabled = 0;

#if defined(WIFI_QUICK_REINIT)
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&old_value);
	if(old_value != disabled)
	{
		quick_reinit = 0;
	}
#endif

	if ( apmib_set( MIB_WLAN_WLAN_DISABLED, (void *)&disabled) == 0) {
		strcpy(tmpBuf, ("Set disabled flag error!"));
		goto setErr_wlan;
	}

	if ( disabled )
		return 0;

#ifdef WIFI_SIMPLE_CONFIG
	memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
	wps_config_info.caller_id = CALLED_FROM_WLANHANDLER;
	apmib_get(MIB_WLAN_SSID, (void *)wps_config_info.ssid); 
	apmib_get(MIB_WLAN_MODE, (void *)&wps_config_info.wlan_mode);
#endif

	sprintf(varName, "regdomain%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");
	if(strVal[0]){
		apmib_get(MIB_HW_REG_DOMAIN, (void *)&val);
		if(val != atoi(strVal)){
			val=atoi(strVal);
			if ( apmib_set(MIB_HW_REG_DOMAIN, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set wlan regdomain error!"));
					goto setErr_wlan;
			}
			apmib_update(HW_SETTING);
		}
	}

	sprintf(varName, "countrystr%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");
	if(strVal[0]){
			if (apmib_set(MIB_WLAN_COUNTRY_STRING, (void *)strVal) == 0) {
					strcpy(tmpBuf, ("Set wlan countrystr error!"));
					goto setErr_wlan;
			}
	}
	
	sprintf(varName, "mode%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");

	if(strVal[0] == 0)
	{
		int val;

		apmib_get( MIB_WLAN_MODE, (void *)&val);
		sprintf(strtmp,"%d",val);
		strVal = strtmp;		
	}


	if ( strVal[0] ) {
#ifndef CONFIG_RTK_MESH
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3'
#ifdef CONFIG_RTL_P2P_SUPPORT
		&&	strVal[0]!= '8'
#endif
		) 
#else
#ifdef CONFIG_NEW_MESH_UI
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3' &&  strVal[0]!= '4' &&  strVal[0]!= '5' ) 
#else
		if (strVal[0]!= '0' && strVal[0]!= '1' && strVal[0]!= '2' &&  strVal[0]!= '3' &&  strVal[0]!= '4' &&  strVal[0]!= '5' &&  strVal[0]!= '6'&&  strVal[0]!= '7') 
#endif
#endif // CONFIG_RTK_MESH
		{
			printf("%c\n",strVal[0]);
			strcpy(tmpBuf, ("Invalid mode value!"));
			goto setErr_wlan;
		}
		*mode = strVal[0] - '0';
#if defined(WIFI_QUICK_REINIT)
		int op_mode = 0;
		apmib_get(MIB_WLAN_MODE, (void *)&old_value);
		apmib_get(MIB_OP_MODE, (void *)&op_mode);
		if(old_value != *mode)
		{
			quick_reinit = 0;
		}
		
		if(old_value != *mode && op_mode == WISP_MODE 
			&& (*mode == CLIENT_MODE || *mode == AP_MODE)){
			MUST_REBOOT = 1;
		}
#endif
		if (*mode == CLIENT_MODE) {
			ENCRYPT_T encrypt;
			apmib_get( MIB_WLAN_ENCRYPT,  (void *)&encrypt);

			if (encrypt &  ENCRYPT_WPA2_MIXED) {
				int format;
				apmib_get( MIB_WLAN_WPA_AUTH, (void *)&format);
				
				if (format & 1) { // radius
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					//Support wlan client mode with Enterprise (RADIUS)
#else
					strcpy(tmpBuf, ("You cannot set client mode with Enterprise (RADIUS) !<br><br>Please change the encryption method in security page first."));
					goto setErr_wlan;
#endif
				}
				
			}
			else if (encrypt == ENCRYPT_WEP || encrypt == 0) {
				int use1x;
				apmib_get( MIB_WLAN_ENABLE_1X, (void *)&use1x);
				if (use1x & 1) {
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					//Support wlan client mode with Enterprise (RADIUS)
#else
					strcpy(tmpBuf, ("You cannot set client mode with 802.1x enabled!<br><br>Please change the encryption method in security page first."));
					goto setErr_wlan;
#endif
				}
			}

			sprintf(varName, "wlanMacClone%d", wlan_id);
			strVal =get_cstream_var(postData,len, varName, "");
			if ( !strcmp(strVal, "ON"))
				val = 1 ;
			else
				val = 0 ;
			if ( apmib_set( MIB_WLAN_MACCLONE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set wlan Mac clone error!"));
				goto setErr_wlan;
			}

		}

		if ( apmib_set( MIB_WLAN_MODE, (void *)mode) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_MODE error!"));
			goto setErr_wlan;
		}

#ifdef WLAN_EASY_CONFIG
		apmib_set( MIB_WLAN_EASYCFG_WLAN_MODE, (void *)mode);
#endif

	}

	sprintf(varName, "ssid%d", wlan_id);
	strSSID = get_cstream_var(postData,len, varName, "");
	if ( strSSID[0] ) {
		if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
				strcpy(tmpBuf, ("Set SSID error!"));
				goto setErr_wlan;
		}
	}
	else if ( *mode == 1 && !strSSID[0] ) { // client and NULL SSID
		if ( apmib_set(MIB_WLAN_SSID, (void *)strSSID) == 0) {
				strcpy(tmpBuf, ("Set SSID error!"));
				goto setErr_wlan;
		}
	}

	sprintf(varName, "chan%d", wlan_id);
	strChan = get_cstream_var(postData,len, varName, "");
	if ( strChan[0] ) {
		errno=0;
		chan = strtol( strChan, (char **)NULL, 10);
		if (errno) {
			strcpy(tmpBuf, ("Invalid channel number!"));
			goto setErr_wlan;
		}
		if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0) {
			strcpy(tmpBuf, ("Set channel number error!"));
			goto setErr_wlan;
		}
	}
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	else {
		//sprintf(varName, "cmj_channel%d", wlan_id);
		strChan = get_cstream_var(postData,len, "cmj_channel", "");
		if ( strChan[0] ) {
			errno=0;
			chan = strtol( strChan, (char **)NULL, 10);
			if (errno) {
				strcpy(tmpBuf, ("Invalid channel number!"));
				goto setErr_wlan;
			}
			if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0) {
				strcpy(tmpBuf, ("Set channel number error!"));
				goto setErr_wlan;
			}
		}
	}
#endif

	sprintf(varName, "type%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");
	if (strVal[0]) {
		if (strVal[0]!= '0' && strVal[0]!= '1') {
			strcpy(tmpBuf, ("Invalid network type value!"));
			goto setErr_wlan;
		}
		if (strVal[0] == '0')
			net = INFRASTRUCTURE;
		else
			net = ADHOC;
		if ( apmib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_NETWORK_TYPE failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "band%d", wlan_id);
	strVal = get_cstream_var(postData,len ,varName, "");
	if ( strVal[0] ) 
	{
		int wlan_onoff_tkip;
		
		apmib_get( MIB_WLAN_11N_ONOFF_TKIP, (void *)&wlan_onoff_tkip);
				
		band_no = strtol( strVal, (char **)NULL, 10);
		if (band_no < 0 || band_no > 78) { //8812
			strcpy(tmpBuf, ("Invalid band value!"));
			goto setErr_wlan;
		}
		//val = (strVal[0] - '0' + 1);
		if(wlan_onoff_tkip == 0) //Wifi request
		{
			int wpaCipher;
			int wpa2Cipher;
			int wdsEncrypt;
			int wlan_encrypt=0;
			int encrypt;
			
			apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
			apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
			apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
			apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
			
			if(*mode != CLIENT_MODE && (band_no == 7 || band_no == 9 || band_no == 10 || band_no == 11 || band_no == 75|| band_no ==63|| band_no ==71||band_no == 67)) //7:n; 9:gn; 10:bgn 11:5g_an
			{
				
				if(wlan_encrypt ==ENCRYPT_WPA || wlan_encrypt ==ENCRYPT_WPA2){
				wpaCipher &= ~WPA_CIPHER_TKIP;
					if(wpaCipher== 0)
						wpaCipher =  WPA_CIPHER_AES;
				//apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
				
				wpa2Cipher &= ~WPA_CIPHER_TKIP;
					if(wpa2Cipher== 0)
						wpa2Cipher =  WPA_CIPHER_AES;
				apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
				
				encrypt=WSC_ENCRYPT_AES;
				apmib_set(MIB_WLAN_WSC_ENC, (void *)&encrypt);
				}
				if(wdsEncrypt == WDS_ENCRYPT_TKIP)
				{
					wdsEncrypt = WDS_ENCRYPT_DISABLED;
					apmib_set( MIB_WLAN_WDS_ENCRYPT, (void *)&wdsEncrypt);
				}
			}		
		}
		val = (band_no + 1);
		if ( apmib_set( MIB_WLAN_BAND, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set band error!"));
			goto setErr_wlan;
		}
		//change vap band when root ap band not include vap band
		int root_ap_band = val;
		int vap_idx,vap_val,tmp_i;
		int old_vwlan_idx = apmib_get_vwlanidx();
		for (vap_idx=1; vap_idx<=4; vap_idx++)
		{
			apmib_set_vwlanidx(vap_idx); 
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&vap_val);
			if(!vap_val)//ON
			{
				apmib_get(MIB_WLAN_BAND, (void *)&vap_val);
				if(((vap_val&1) && (!(root_ap_band&1))) || ((vap_val&2) && (!(root_ap_band&2))) || ((vap_val&8) && (!(root_ap_band&8))) || ((vap_val&4) && (!(root_ap_band&4))) || ((vap_val&64) && (!(root_ap_band&64))))
				{
					apmib_set(MIB_WLAN_BAND, (void *)&root_ap_band);
					vap_val = 0;
					apmib_set(MIB_WLAN_BASIC_RATES, (void *)&vap_val);		
					apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&vap_val);
					if(root_ap_band&8)//11N, no TKIP
					{
						int wpaCipher;
						int wpa2Cipher;
						int wlan_encrypt;
						apmib_get(MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
						apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
						apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
						if(wlan_encrypt ==ENCRYPT_WPA || wlan_encrypt ==ENCRYPT_WPA2 || wlan_encrypt == ENCRYPT_WPA2_MIXED){
							wpaCipher &= ~WPA_CIPHER_TKIP;
							if(wpaCipher== 0)
								wpaCipher =  WPA_CIPHER_AES;
							//apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
							
							wpa2Cipher &= ~WPA_CIPHER_TKIP;
							if(wpa2Cipher== 0)
								wpa2Cipher =  WPA_CIPHER_AES;
							apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
						}
					}
				}
			}
		}
		apmib_set_vwlanidx(old_vwlan_idx); 
		
#if defined(CONFIG_RTL_8812_SUPPORT)
		{
			int band2G5GSelect = 0;
			strVal = get_cstream_var(postData,len, "Band2G5GSupport", ""); //wlan0 PHYBAND_TYPE
			if(strVal[0] != 0)
			{
				band2G5GSelect = atoi(strVal);
				printf("band2G5GSelect = %d\n", band2G5GSelect);
			}
			else
				band2G5GSelect = wlan_id ? 1:2;

			if(band_no==3 || band_no==11 || band_no==75 || band_no ==63 || band_no ==71 || band_no == 67)
				val = 2;
			else if(band_no==7)
			{
				val = band2G5GSelect;
			}
			else
				val = 1;
			if(wlanBand2G5GSelect == BANDMODESINGLE){
				int idx,old_vwlan_idx,band ;
				band = band_no+1;
				apmib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&old_value);
				if(val != old_value){// switch band
					old_vwlan_idx = apmib_get_vwlanidx();
					for (idx=1; idx<=4; idx++) {		
						apmib_set_vwlanidx(idx);
						apmib_set( MIB_WLAN_BAND, (void *)&band);
						apmib_set_vwlanidx(old_vwlan_idx);
					}
					apmib_set_vwlanidx(old_vwlan_idx);
				}
			}
			if ( apmib_set( MIB_WLAN_PHY_BAND_SELECT, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set band error!"));
					goto setErr_wlan;
			}
		}
#endif
	}

	// set tx rate
	sprintf(varName, "txRate%d", wlan_id);
	strRate = get_cstream_var(postData,len,varName, "");
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			val = 1;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_wlan;
			}
		}
		else  {
			val = 0;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_wlan;
			}  
			val = atoi(strRate);

			if(val<30)
			val = 1 << (val-1);
			else
				val = ((1<<31) + (val -30));
			
			if ( apmib_set(MIB_WLAN_FIX_RATE, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set fix rate failed!"));
				goto setErr_wlan;
			}
		}			
	}

	sprintf(varName, "basicrates%d", wlan_id);
	strRate = get_cstream_var(postData,len,varName, ""); 
	if ( strRate[0] ) {
		val = atoi(strRate);		
		if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx basic rate failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "operrates%d", wlan_id);
	strRate = get_cstream_var(postData,len, varName, ""); 
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx operation rate failed!"));
			goto setErr_wlan;
		}
	}

	// set hidden SSID
	sprintf(varName, "hiddenSSID%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set hidden ssid failed!"));
			goto setErr_wlan;
		}
	}
	sprintf(varName, "wlanwmm%d", wlan_id);
	strVal= get_cstream_var(postData,len,varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid WMM value."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
			goto setErr_wlan;
		}
	}else{
		//enable wmm in 11N mode always
			apmib_get( MIB_WLAN_BAND, (void *)&cur_band);
			if(cur_band == 10 || cur_band ==11){
				val = 1;
				if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
					goto setErr_wlan;
				}
			}
	}
// for 11N
	sprintf(varName, "channelbound%d", wlan_id);
	strVal = get_cstream_var(postData,len, varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if (strVal[0] == '1')
			val = 1;
		else if (strVal[0] == '2') //8812
		{
			val = 2;
		}
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CHANNEL_BONDING failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "controlsideband%d", wlan_id);
	strVal= get_cstream_var(postData,len,varName, "");
	if (strVal[0]) {
		if ( strVal[0] == '0')
			val = 0;
		else if ( strVal[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Control SideBand."));
			goto setErr_wlan;
		}
		if ( apmib_set(MIB_WLAN_CONTROL_SIDEBAND, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CONTROL_SIDEBAND failed!"));
			goto setErr_wlan;
		}
	}

//

	sprintf(varName, "basicrates%d", wlan_id);
	strRate = get_cstream_var(postData,len, varName, "");
	if ( strRate[0] ) {
		val = atoi(strRate);

		if ( val && apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx basic rate failed!"));
			goto setErr_wlan;
		}
	}

	sprintf(varName, "operrates%d", wlan_id);
	strRate =get_cstream_var(postData,len, varName, "");
	if ( strRate[0] ) {
		val = atoi(strRate);
		if ( val && apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Tx operation rate failed!"));
			goto setErr_wlan;
		}
	}	//do twice ??
#if defined(CONFIG_RTL_8881A_SELECTIVE)
	{
		int idx;
		int i, val, val2;
	
		for (idx=2;idx<=2;idx++) {
	
			sprintf(varName, "channel_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
	
				if (apmib_set(idx==1?MIB_WLAN_CHANNEL:MIB_WLAN_CHANNEL_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
	
			sprintf(varName, "ChannelBonding_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
	
				if ( val && apmib_set(idx==1?MIB_WLAN_CHANNEL_BONDING:MIB_WLAN_CHANNEL_BONDING_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
	
			sprintf(varName, "ControlSideBand_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
				printf("=======ControlSideBand is %d, idx is %d sizeof(wlan)=%d sizeof(apmib)=%d\n", val, idx,sizeof(CONFIG_WLAN_SETTING_T),sizeof(APMIB_T));
				if ( apmib_set(idx==1?MIB_WLAN_CONTROL_SIDEBAND:MIB_WLAN_CONTROL_SIDEBAND_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
	
			sprintf(varName, "operRate_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
	
				if ( val && apmib_set(idx==1?MIB_WLAN_SUPPORTED_RATES:MIB_WLAN_SUPPORTED_RATES_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
	
			sprintf(varName, "basicRate_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
	
				if ( val && apmib_set(idx==1?MIB_WLAN_BASIC_RATES:MIB_WLAN_BASIC_RATES_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
	
			sprintf(varName, "coexist_%d", idx);
			strRate = get_cstream_var(postData,len, varName, "");
			if ( strRate[0] ) {
				val = atoi(strRate);
	
				if (apmib_set(idx==1?MIB_WLAN_COEXIST_ENABLED:MIB_WLAN_COEXIST_ENABLED_2, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_wlan;
				}
			}
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
#ifdef CONFIG_RTK_MESH
	if( *mode >= 4 && *mode <=7)
	{
		val=0;
		apmib_set(MIB_REPEATER_ENABLED1, (void *)&val);
		apmib_set(MIB_REPEATER_ENABLED2, (void *)&val);
	}
	else
#endif
	{	int id;
		sprintf(varName, "repeaterEnabled%d", wlan_id);
		strVal = get_cstream_var(postData,len, ("lan_ip"), "");
		
		if ((strVal==NULL || strVal[0]==0)	 // not called from wizard	
				//&& (*mode != WDS_MODE)	&& (*mode != CLIENT_MODE)
			) 
			{
			int rpt_enabled;
			
			strVal = get_cstream_var(postData,len, varName, "");
			if ( !strcmp(strVal, "ON")) {
				val = 1 ;
            }else{
#ifdef  RTL_MULTI_REPEATER_MODE_SUPPORT		
                if (wlan_id == 0)
                    id = MIB_REPEATER_ENABLED1;
			else
                    id = MIB_REPEATER_ENABLED2;

                apmib_get(id, (void *)&rpt_enabled);

                if(rpt_enabled==2){
                    val = 2 ;
                }else
#endif
		        {
				val = 0 ;
                }
            }
				
#if defined(CONFIG_RTL_ULINKER)
			if (wlan_id == 0)
				id = MIB_REPEATER_ENABLED1;
			else
				id = MIB_REPEATER_ENABLED2;
			apmib_get(id, (void *)&rpt_enabled);
			if(*mode == AP_MODE && rpt_enabled == 1) //ulinker repeater mode
			{
				val = 1;
			}
#endif
				
			if (wlan_id == 0)
				id = MIB_REPEATER_ENABLED1;
			else
				id = MIB_REPEATER_ENABLED2;
#if defined(WIFI_QUICK_REINIT)
			apmib_get(id, (void *)&old_value);
			if(old_value != val)
			{
				quick_reinit = 0;
			}
#endif
			apmib_set(id, (void *)&val);

			if (val == 1) {
				sprintf(varName, "repeaterSSID%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal){
					if (wlan_id == 0)
						id = MIB_REPEATER_SSID1;
					else
						id = MIB_REPEATER_SSID2;
						
					setRepeaterSsid(wlan_id, id, strVal);
				}
			}

#ifdef MBSSID
			int old_idx = apmib_get_vwlanidx();
			apmib_set_vwlanidx(NUM_VWLAN_INTERFACE); // repeater interface
			int disable;
			if (val)
				disable = 0;
			else
				disable = 1;	
#if 0 //defined(WIFI_QUICK_REINIT)
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&old_value);
			if(old_value != val)
			{
				quick_reinit = 0;
			}
#endif
			
			apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&disable);

			if (!disable) {
				if (*mode == CLIENT_MODE)
					val = AP_MODE;
				else
					val = CLIENT_MODE;
				apmib_set(MIB_WLAN_MODE, (void *)&val); 		
				if(strVal)
					apmib_set(MIB_WLAN_SSID, (void *)strVal);			
			}

			if (val == CLIENT_MODE) {
				// if client mode, check if Radius or mixed mode encryption is used
				apmib_get(MIB_WLAN_ENCRYPT, (void *)&val);

				if (val <= ENCRYPT_WEP) {				
					apmib_get( MIB_WLAN_ENABLE_1X, (void *)&val);
					if (val != 0) {
						val = 0;
						apmib_set( MIB_WLAN_ENABLE_1X, (void *)&val);				
					}
				}	
				else if (val == ENCRYPT_WPA2_MIXED) {				
					val = ENCRYPT_DISABLED;
					apmib_set(MIB_WLAN_ENCRYPT, (void *)&val);
				}
				else if (val == ENCRYPT_WPA) {	
					apmib_get(MIB_WLAN_WPA_AUTH, (void *)&val);
					if ((val == 0) || (val & 1)) { // if no or radius, force to psk
						val = 2;
						apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val);
					}				
					apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val);
					if ((val == 0) || (val == WPA_CIPHER_MIXED)) {
						val = WPA_CIPHER_AES;
    					//apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&val);					
					}
				}
				else if (val == ENCRYPT_WPA2) { 
					apmib_get(MIB_WLAN_WPA_AUTH, (void *)&val);
					if ((val == 0) || (val & 1)) { // if no or radius, force to psk
						val = 2;
						apmib_set(MIB_WLAN_WPA_AUTH, (void *)&val);
					}				
					apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val);
					if ((val == 0) || (val == WPA_CIPHER_MIXED)) {
						val = WPA_CIPHER_AES;
						apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&val);					
					}
				}	
			}

			apmib_set_vwlanidx(old_idx);
#endif	
		}
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
	strVal =get_cstream_var(postData,len, varName, "");
	val = 0;
	if (strVal && strVal[0])
		val = atoi(strVal);
	update_wps_configured(val);
#endif

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	set_wps_interface();
#endif

#if defined(WIFI_QUICK_REINIT)
	return quick_reinit;
#else
	return 0;
#endif

setErr_wlan:
	return -1 ;
}
extern void set_wps_interface();

void formWlanSetup(char *postData, int len)
{
	char *submitUrl;
	char tmpBuf[150];
	int mode=-1;
	int warn=0;
	int val;	
	char *strVal=NULL;
	int wlan_idx=0;
#if defined(CONFIG_RTL_92D_SUPPORT)
	int wlanif=0;
	
	PHYBAND_TYPE_T phyBandSelect = PHYBAND_OFF; 
	int wlanBand2G5GSelect=PHYBAND_OFF;
#endif
	int ret = 0;
#if defined(WIFI_QUICK_REINIT)
	int quick_reinit = 0;
#endif

//displayPostDate(wp->post_data);	

	wlan_idx=apmib_get_wlanidx();
#if defined(CONFIG_RTL_92D_SUPPORT)		
	apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);

	if(wlanBand2G5GSelect == BANDMODESINGLE)
	{

		
		strVal=get_cstream_var(postData,len,("Band2G5GSupport"),"");
		if(strVal[0])
		{
			
			phyBandSelect= atoi(strVal);		
			wlanif = whichWlanIfIs(phyBandSelect);			
				

			if(wlanif != 0)
			{

				val = 1;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //close original interface
				
				swapWlanMibSetting(0,wlanif);
				
				val = 0;
				apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val); //enable after interface
				//apmib_update_web(CURRENT_SETTING);
			}
		}
		
#if defined(CONFIG_RTL_P2P_SUPPORT)		
		char varName[20];
		sprintf(varName, "mode%d", wlan_idx);
		strVal = get_cstream_var(postData,len, varName, "");
	
		if(strVal[0] == NULL)
		{
			char strtmp[20];
			
			apmib_get( MIB_WLAN_MODE, (void *)&val);
			sprintf(strtmp,"%d",val);
			strVal = strtmp;		
		}
		
		val=atoi(strVal);
		
		if(val == P2P_SUPPORT_MODE)
		{			
			int val2 = 0;
#ifdef CONFIG_WLANIDX_MUTEX
			int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif
			apmib_set( MIB_DHCP, (void *)&val2);
			
			apmib_set_wlanidx(0);
			apmib_set( MIB_WLAN_MODE, (void *)&val);
			
			apmib_set_wlanidx(1);
			apmib_set( MIB_WLAN_MODE, (void *)&val);			
			
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif

		}
#endif //#if defined(CONFIG_RTL_P2P_SUPPORT)
		
	}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)

#if defined(WLAN_PROFILE)
	if(wlProfileHandler(postData,len, tmpBuf, wlan_idx) < 0)
	{
		goto setErr_wlan;
	}

#endif //#if defined(WLAN_PROFILE)	
	ret = wlanHandler(postData,len, tmpBuf, &mode, wlan_idx);
	if(ret < 0)
		goto setErr_wlan ;
#if defined(WIFI_QUICK_REINIT)
	else if(ret == 1)
	{
		quick_reinit = 1;
	}
#endif
	
	if (mode == 1) { // not AP mode
		//set cipher suit to AES + tkip if wpa2 mixed mode is set
		ENCRYPT_T encrypt;
		int intVal;
		apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
		if(encrypt == ENCRYPT_WPA2_MIXED){
			intVal =   WPA_CIPHER_MIXED ;
			
			if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_wlan;
			}
			intVal =   WPA_CIPHER_MIXED;
			if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_wlan;
			}
		}
	}
	if (mode != 1) { // AP mode
		//set wpa2-aes  if wpa mode is set
		ENCRYPT_T encrypt;
		int intVal;
		apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
		if(encrypt == ENCRYPT_WPA){
			intVal =   WPA_CIPHER_AES ;
			if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_wlan;
			}
			encrypt = ENCRYPT_WPA2;
			if ( apmib_set(MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_wlan;
			}
			strcpy(tmpBuf, ("Warning! WPA encryption is only supported in client Mode. <BR> Change to WPA2 Encryption."));
			warn = 1;
		}
	}
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	//run_init_script("bridge");
	
#endif

	submitUrl = get_cstream_var(postData,len, ("wlan-url"), "");   // hidden page
	
	if (warn) {
		OK_MSG1(tmpBuf, submitUrl);
	}
	else {
#ifdef HAVE_SYSTEM_REINIT
			{
				int opmode=0;
				sprintf(tmpBuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,wlan_idx);
				apmib_get(MIB_OP_MODE, (void *)&opmode);
				if(opmode == WISP_MODE)
					wait_redirect("Apply Changes", 25,tmpBuf);
				else
					wait_redirect("Apply Changes", 10,tmpBuf);
				sleep(1);
				if(opmode == WISP_MODE)
					kick_reinit_m(SYS_REINIT_ALL);
				else
					kick_reinit_m(SYS_WIFI_M|SYS_BRIDGE_M|SYS_WIFI_APP_M);
			}
#elif defined(WIFI_QUICK_REINIT)
			if(MUST_REBOOT != 1)
			{
				kick_event(WLAN_BRIDGE_EVENT);
				send_redirect_perm(submitUrl);
			}
			else
				OK_MSG(submitUrl);
#else
				OK_MSG(submitUrl);
#endif
	}
	set_wps_interface();

	save_cs_to_file();

	return;

setErr_wlan:
	ERR_MSG(tmpBuf);
}


/**************************************************************/

int advanceHander(char *postData, int len ,char *tmpBuf)
{
	char *strAuth, *strFragTh, *strRtsTh, *strBeacon, *strPreamble, *strAckTimeout, *threshold;
	char *strRate, /* *strHiddenSSID, */ *strDtim, *strIapp, *strProtection,*strHs2;
	char *strTurbo, *strPower;
	char *strValue;
	AUTH_TYPE_T authType;
	PREAMBLE_T preamble;
	int val;

#if defined(WIFI_QUICK_REINIT)
	int old_value, quick_reinit=1;
#endif

#ifdef WIFI_SIMPLE_CONFIG
	memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
	wps_config_info.caller_id = CALLED_FROM_ADVANCEHANDLER;
	apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&wps_config_info.shared_type);
#endif

	strAuth = get_cstream_var(postData,len, ("authType"), "");
	if (strAuth[0]) {
		if ( !strcmp(strAuth, ("open")))
			authType = AUTH_OPEN;
		else if ( !strcmp(strAuth, ("shared")))
			authType = AUTH_SHARED;
		else if ( !strcmp(strAuth, ("both")))
			authType = AUTH_BOTH;
		else {
			strcpy(tmpBuf, ("Error! Invalid authentication value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType) == 0) {
			strcpy(tmpBuf, ("Set authentication failed!"));
			goto setErr_advance;
		}
	}
	strFragTh =get_cstream_var(postData,len, ("fragThreshold"), "");
	if (strFragTh[0]) {
		if ( !string_to_dec(strFragTh, &val) || val<256 || val>2346) {
			strcpy(tmpBuf, ("Error! Invalid value of fragment threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set fragment threshold failed!"));
			goto setErr_advance;
		}
	}
	strRtsTh = get_cstream_var(postData,len, ("rtsThreshold"), "");
	if (strRtsTh[0]) {
		if ( !string_to_dec(strRtsTh, &val) || val<0 || val>2347) {
			strcpy(tmpBuf, ("Error! Invalid value of RTS threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set RTS threshold failed!"));
			goto setErr_advance;
		}
	}

	strBeacon = get_cstream_var(postData,len, ("beaconInterval"), "");
	if (strBeacon[0]) {
		if ( !string_to_dec(strBeacon, &val) || val<20 || val>1024) {
			strcpy(tmpBuf, ("Error! Invalid value of Beacon Interval."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Beacon interval failed!"));
			goto setErr_advance;
		}
	}

#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER)
	threshold = get_cstream_var(postData,len, ("ScGoodRssiThreshold"), "");
	if (threshold[0]) {
		if ( !string_to_dec(threshold, &val) || val<0 || val>100) {
			strcpy(tmpBuf, ("Error! Invalid value of GoodRssiThreshold threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_SC_GOOD_RSSI_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set RTS GoodRssiThreshold failed!"));
			goto setErr_advance;
		}
	}
	threshold = get_cstream_var(postData,len, ("ScNormalRssiThreshold"), "");
	if (threshold[0]) {
		if ( !string_to_dec(threshold, &val) || val<0 || val>100) {
			strcpy(tmpBuf, ("Error! Invalid value of NormalRssiThreshold threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_SC_NORMAL_RSSI_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set NormalRssiThreshold threshold failed!"));
			goto setErr_advance;
		}
	}
	threshold = get_cstream_var(postData,len, ("ScPoorRssiThreshold"), "");
	if (threshold[0]) {
		if ( !string_to_dec(threshold, &val) || val<0 || val>100) {
			strcpy(tmpBuf, ("Error! Invalid value of PoorRssiThreshold threshold."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_SC_POOR_RSSI_THRESHOLD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set PoorRssiThreshold threshold failed!"));
			goto setErr_advance;
		}
	}
#endif //defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER)

	strAckTimeout = get_cstream_var(postData,len, ("ackTimeout"), "");
	if (strAckTimeout[0]) {
		if ( !string_to_dec(strAckTimeout, &val) || val<0 || val>255) {
			strcpy(tmpBuf, ("Error! Invalid value of Ack timeout."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_ACK_TIMEOUT, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set Ack timeout failed!"));
			goto setErr_advance;
		}
	}
#if 0
	// set tx rate
	strRate = req_get_cstream_var(wp, ("txRate"), "");
	if ( strRate[0] ) {
		if ( strRate[0] == '0' ) { // auto
			val = 1;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_advance;
			}
		}
		else  {
			val = 0;
			if ( apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set rate adaptive failed!"));
				goto setErr_advance;
			}  
			val = atoi(strRate);
			val = 1 << (val-1);
			if ( apmib_set(MIB_WLAN_FIX_RATE, (void *)&val) == 0) {
				strcpy(tmpBuf, ("Set fix rate failed!"));
				goto setErr_advance;
			}
			strRate = req_get_cstream_var(wp, ("basicrates"), "");
			if ( strRate[0] ) {
				val = atoi(strRate);
				if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx basic rate failed!"));
					goto setErr_advance;
				}
			}

			strRate = req_get_cstream_var(wp, ("operrates"), "");
			if ( strRate[0] ) {
				val = atoi(strRate);
				if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
					strcpy(tmpBuf, ("Set Tx operation rate failed!"));
					goto setErr_advance;
				}
			}	
		}
	}
#endif
	val = 0;
	strRate = get_cstream_var(postData,len, ("operRate1M"), "");
	if (strRate==NULL || strRate[0]==0)
		goto skip_rate_setting;
	if ( !strcmp(strRate, ("1M")))
		val |= TX_RATE_1M;
	strRate = get_cstream_var(postData,len, ("operRate2M"), "");
	if ( !strcmp(strRate, ("2M")))
		val |= TX_RATE_2M;
	strRate = get_cstream_var(postData,len,("operRate5M"), "");
	if ( !strcmp(strRate, ("5M")))
		val |= TX_RATE_5M;
	strRate = get_cstream_var(postData,len, ("operRate11M"), "");
	if ( !strcmp(strRate, ("11M")))
		val |= TX_RATE_11M;
	strRate = get_cstream_var(postData,len, ("operRate6M"), "");
	if ( !strcmp(strRate, ("6M")))
		val |= TX_RATE_6M;
	strRate = get_cstream_var(postData,len, ("operRate9M"), "");
	if ( !strcmp(strRate, ("9M")))
		val |= TX_RATE_9M;
	strRate = get_cstream_var(postData,len, ("operRate12M"), "");
	if ( !strcmp(strRate, ("12M")))
		val |= TX_RATE_12M;
	strRate = get_cstream_var(postData,len, ("operRate18M"), "");
	if ( !strcmp(strRate, ("18M")))
		val |= TX_RATE_18M;			
	strRate = get_cstream_var(postData,len, ("operRate24M"), "");
	if ( !strcmp(strRate, ("24M")))
		val |= TX_RATE_24M;			
	strRate = get_cstream_var(postData,len, ("operRate36M"), "");
	if ( !strcmp(strRate, ("36M")))
		val |= TX_RATE_36M;			
	strRate = get_cstream_var(postData,len, ("operRate48M"), "");
	if ( !strcmp(strRate, ("48M")))
		val |= TX_RATE_48M;			
	strRate = get_cstream_var(postData,len, ("operRate54M"), "");
	if ( !strcmp(strRate, ("54M")))
		val |= TX_RATE_54M;
	if ( apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val) == 0) {
		strcpy(tmpBuf, ("Set Tx operation rate failed!"));
		goto setErr_advance;
	}

	// set basic tx rate
	val = 0;
	strRate = get_cstream_var(postData,len, ("basicRate1M"), "");
	if (strRate==NULL || strRate[0]==0)
		goto skip_rate_setting;	
	if ( !strcmp(strRate, ("1M")))
		val |= TX_RATE_1M;
	strRate = get_cstream_var(postData,len, ("basicRate2M"), "");
	if ( !strcmp(strRate, ("2M")))
		val |= TX_RATE_2M;
	strRate = get_cstream_var(postData,len, ("basicRate5M"), "");
	if ( !strcmp(strRate, ("5M")))
		val |= TX_RATE_5M;
	strRate = get_cstream_var(postData,len, ("basicRate11M"), "");
	if ( !strcmp(strRate, ("11M")))
		val |= TX_RATE_11M;
	strRate = get_cstream_var(postData,len, ("basicRate6M"), "");
	if ( !strcmp(strRate, ("6M")))
		val |= TX_RATE_6M;
	strRate = get_cstream_var(postData,len, ("basicRate9M"), "");
	if ( !strcmp(strRate, ("9M")))
		val |= TX_RATE_9M;
	strRate = get_cstream_var(postData,len, ("basicRate12M"), "");
	if ( !strcmp(strRate, ("12M")))
		val |= TX_RATE_12M;
	strRate = get_cstream_var(postData,len, ("basicRate18M"), "");
	if ( !strcmp(strRate, ("18M")))
		val |= TX_RATE_18M;			
	strRate = get_cstream_var(postData,len, ("basicRate24M"), "");
	if ( !strcmp(strRate, ("24M")))
		val |= TX_RATE_24M;			
	strRate = get_cstream_var(postData,len, ("basicRate36M"), "");
	if ( !strcmp(strRate, ("36M")))
		val |= TX_RATE_36M;			
	strRate = get_cstream_var(postData,len, ("basicRate48M"), "");
	if ( !strcmp(strRate, ("48M")))
		val |= TX_RATE_48M;			
	strRate = get_cstream_var(postData,len, ("basicRate54M"), "");
	if ( !strcmp(strRate, ("54M")))
		val |= TX_RATE_54M;			
	if ( apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val) == 0) {
		strcpy(tmpBuf, ("Set Tx basic rate failed!"));
		goto setErr_advance;
	}		
skip_rate_setting:
	// set preamble
	strPreamble = get_cstream_var(postData,len, ("preamble"), "");
	if (strPreamble[0]) {
		if (!strcmp(strPreamble, ("long")))
			preamble = LONG_PREAMBLE;
		else if (!strcmp(strPreamble, ("short")))
			preamble = SHORT_PREAMBLE;
		else {
			strcpy(tmpBuf, ("Error! Invalid Preamble value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&preamble) == 0) {
			strcpy(tmpBuf, ("Set Preamble failed!"));
			goto setErr_advance;
		}
	}
//move to basic setting page
#if 0
	// set hidden SSID
	strHiddenSSID = req_get_cstream_var(wp, ("hiddenSSID"), "");
	if (strHiddenSSID[0]) {
		if (!strcmp(strHiddenSSID, ("no")))
			val = 0;
		else if (!strcmp(strHiddenSSID, ("yes")))
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid hiddenSSID value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set hidden ssid failed!"));
			goto setErr_advance;
		}
	}
#endif
	strDtim = get_cstream_var(postData,len, ("dtimPeriod"), "");
	if (strDtim[0]) {
		if ( !string_to_dec(strDtim, &val) || val<1 || val>255) {
			strcpy(tmpBuf, ("Error! Invalid value of DTIM period."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_DTIM_PERIOD, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set DTIM period failed!"));
			goto setErr_advance;
		}
	}

	strIapp = get_cstream_var(postData,len, ("iapp"), "");
	if (strIapp[0]) {
		if (!strcmp(strIapp, ("no")))
			val = 1;
		else if (!strcmp(strIapp, ("yes")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid IAPP value."));
			goto setErr_advance;
		}
#if defined(WIFI_QUICK_REINIT)
		apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&old_value);
		if(val != old_value){
			quick_reinit = 0;
			MUST_REBOOT = 1;
		}
		
#endif
		if ( apmib_set(MIB_WLAN_IAPP_DISABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_IAPP_DISABLED failed!"));
			goto setErr_advance;
		}
	}
#ifdef HAVE_HS2_SUPPORT	
	strHs2 = get_cstream_var(postData,len, ("hs2"), "");
	if (strHs2[0]) {
		if (!strcmp(strHs2, ("no")))
			val = 0;
		else if (!strcmp(strHs2, ("yes")))
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid HS2 value."));
			goto setErr_advance;
		}
#if defined(WIFI_QUICK_REINIT)
		apmib_get(MIB_WLAN_HS2_ENABLE, (void *)&old_value);
		if(val != old_value){
			quick_reinit = 0;
			MUST_REBOOT = 1;
		}
#endif
		if ( apmib_set(MIB_WLAN_HS2_ENABLE, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_HS2_ENABLED failed!"));
			goto setErr_advance;
		}
	}
#endif
	strProtection= get_cstream_var(postData,len, ("11g_protection"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("no")))
			val = 1;
		else if (!strcmp(strProtection, ("yes")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid 11g Protection value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_PROTECTION_DISABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_PROTECTION_DISABLED failed!"));
			goto setErr_advance;
		}
	}
#if 0	
// for WMM move to basic setting

	strProtection= req_get_cstream_var(wp, ("wmm"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("on")))
			val = 1;
		else if (!strcmp(strProtection, ("off")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid WMM value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_WMM_ENABLED failed!"));
			goto setErr_advance;
		}
	}
#endif	
	strTurbo = get_cstream_var(postData,len, ("turbo"), "");
	if (strTurbo[0]) {
		if (!strcmp(strTurbo, ("off")))
			val = 2;
		else if (!strcmp(strTurbo, ("always")))
			val = 1;
		else if (!strcmp(strTurbo, ("auto")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid turbo mode value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_TURBO_MODE, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_TURBO_MODE failed!"));
			goto setErr_advance;
		}
	}

	strPower= get_cstream_var(postData,len, ("RFPower"), "");
	if (strPower[0]) {		
		if (!strcmp(strPower, ("0")))
			val = 0;
		else if (!strcmp(strPower, ("1")))
			val = 1;
		else if (!strcmp(strPower, ("2")))
			val = 2;
		else if (!strcmp(strPower, ("3")))
			val = 3;
		else if (!strcmp(strPower, ("4")))
			val = 4;
		else {
			strcpy(tmpBuf, ("Error! Invalid RF output power value."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_RFPOWER_SCALE, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_RFPOWER_SCALE failed!"));
			goto setErr_advance;
		}
	}
#if 0
// for 11N
	strProtection= req_get_cstream_var(wp, ("channelBond0"), "");
	if (strProtection[0]) {
		if ( strProtection[0] == '0')
			val = 0;
		else if (strProtection[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Channel Bonding."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CHANNEL_BONDING failed!"));
			goto setErr_advance;
		}
	}

	strProtection= req_get_cstream_var(wp, ("sideBand0"), "");
	if (strProtection[0]) {
		if ( strProtection[0] == '0')
			val = 0;
		else if ( strProtection[0] == '1')
			val = 1;
		else {
			strcpy(tmpBuf, ("Error! Invalid Control SideBand."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_CONTROL_SIDEBAND, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_CONTROL_SIDEBAND failed!"));
			goto setErr_advance;
		}
	}
#endif	
	strProtection= get_cstream_var(postData,len, ("aggregation"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("disable")))
			val = DISABLED;	// GANTOE & epopen: DISABLED=0 original is DISABLE=0, Because conflict with ../../auth/include/1x_common.h in AP/net-snmp-5.x.x
		else
			val = A_MIXED;	
		if ( apmib_set(MIB_WLAN_AGGREGATION, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_AGGREGATION failed!"));
			goto setErr_advance;
		}
	}
	strValue = get_cstream_var(postData,len, ("block_relay"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;			
			apmib_set(MIB_WLAN_BLOCK_RELAY, (void *)&val);
		}
	strProtection= get_cstream_var(postData,len, ("shortGI0"), "");
	if (strProtection[0]) {
		if (!strcmp(strProtection, ("on")))
			val = 1;
		else if (!strcmp(strProtection, ("off")))
			val = 0;
		else {
			strcpy(tmpBuf, ("Error! Invalid short GI."));
			goto setErr_advance;
		}
		if ( apmib_set(MIB_WLAN_SHORT_GI, (void *)&val) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_SHORT_GI failed!"));
			goto setErr_advance;
		}
	}

	strValue = get_cstream_var(postData,len, ("tx_stbc"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_STBC_ENABLED, (void *)&val);	
		}
		else
		{		
			int chipVersion = getWLAN_ChipVersion();
			if(chipVersion == 1)
			{
				val = 0;	
				apmib_set(MIB_WLAN_STBC_ENABLED, (void *)&val);	
			}


		}
	strValue =get_cstream_var(postData,len, ("coexist_"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_COEXIST_ENABLED, (void *)&val);	
		}

		//### add by sen_liu 2011.3.29 TX Beamforming update to mib in 92D 
		strValue = get_cstream_var(postData,len, ("beamforming_"), "");
		if (strValue[0]) {
			if (!strcmp(strValue, ("enable")))
				val = 1;
			else
				val = 0;	
			apmib_set(MIB_WLAN_TX_BEAMFORMING, (void *)&val);	
		}
		//### end
#ifdef WIFI_SIMPLE_CONFIG
	update_wps_configured(1);
#endif
#if defined(WIFI_QUICK_REINIT)
	return quick_reinit;
#else
	return 0;
#endif
setErr_advance:
	return -1 ;		
}	

void formAdvanceSetup(char *postData, int len)
{

	char tmpBuf[100];
	char *submitUrl;
	char urlbuf[60];
	int ret = 1, quick_reinit = 1;

	submitUrl = get_cstream_var(postData,len, ("submit-url"), "");   // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
	ret = advanceHander(postData,len ,tmpBuf);
	if(ret < 0)
		goto setErr_end;
	else 
		quick_reinit = ret;
	
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif


#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes",5,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M|SYS_WIFI_APP_M);
	}
#elif defined(WIFI_QUICK_REINIT)
	if(MUST_REBOOT != 1)
	{
		kick_event(WLAN_BRIDGE_EVENT);
		send_redirect_perm(urlbuf);
	}
	else
		OK_MSG(submitUrl);
#else
		OK_MSG(submitUrl);
#endif

	save_cs_to_file();

  	return;

setErr_end:
	ERR_MSG(submitUrl);
}

/*********************************************************/


int wpaHandler(char *postData, int len, char *tmpBuf, int wlan_id)
{
   	char *strEncrypt, *strVal;
	ENCRYPT_T encrypt;
	int enableRS=0, intVal, getPSK=0, length, val;
	unsigned long reKeyTime;
	SUPP_NONWAP_T suppNonWPA;
	struct in_addr inIp;
	char varName[20];
#ifdef CONFIG_RTL_WAPI_SUPPORT
	int enableAS=0;
#endif

#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	int wlan_mode;
	int intVal2;
	int wlanIdx_5G, wlanIdx_2G, rsBandSel;
#endif

#if defined(WIFI_QUICK_REINIT)
	int old_value, quick_reinit=1;
	apmib_get(MIB_WLAN_ENABLE_1X, (void *)&old_value);
#endif

	sprintf(varName, "method%d", wlan_id);
   	strEncrypt = get_cstream_var(postData,len, varName, "");
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, ("Error! no encryption method."));
		goto setErr_encrypt;
	}
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
	if (encrypt!=ENCRYPT_DISABLED && encrypt!=ENCRYPT_WEP && encrypt!=ENCRYPT_WPA
		&& encrypt != ENCRYPT_WPA2 && encrypt != ENCRYPT_WPA2_MIXED
#ifdef CONFIG_RTL_WAPI_SUPPORT		
		&& encrypt != ENCRYPT_WAPI
#endif
) {
		strcpy(tmpBuf, ("Invalid encryption method!"));
		goto setErr_encrypt;
	}

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WPAHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wps_config_info.wpa_enc);
		apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wps_config_info.wpa2_enc);
		apmib_get(MIB_WLAN_WPA_PSK, (void *)wps_config_info.wpaPSK);
	}
#endif

	if (apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
  		strcpy(tmpBuf, ("Set MIB_WLAN_ENCRYPT mib error!"));
		goto setErr_encrypt;
	}

	if (encrypt == ENCRYPT_DISABLED || encrypt == ENCRYPT_WEP) {
		sprintf(varName, "use1x%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if ( !strcmp(strVal, "ON")) {
			apmib_get( MIB_WLAN_MODE, (void *)&intVal);
			if (intVal !=AP_MODE && intVal != AP_WDS_MODE) { // not AP mode
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
				if(intVal == CLIENT_MODE){//client mode
//					printf("%s(%d): WPA-RADIUS can be used when device is set to client mode\n",__FUNCTION__,__LINE__);//Added for test 
					intVal = 1;
					enableRS = 1;
				}
				else{
					strcpy(tmpBuf, ("Error! 802.1x authentication cannot be used when device is set to wds or mesh mode."));
					goto setErr_encrypt;
					intVal = 0;
				}
#else
				strcpy(tmpBuf, ("Error! 802.1x authentication cannot be used when device is set to client mode."));
				goto setErr_encrypt;
				//intVal = 0;				
#endif
			}
			else {
				intVal = 1;
				enableRS = 1;
			}
		}
		else
			intVal = 0;
#ifdef HAVE_HS2_SUPPORT
		if(!enableRS) {
			if ( apmib_set( MIB_WLAN_HS2_ENABLE, (void *)&intVal) == 0) {
	  			strcpy(tmpBuf, ("Set hs2 enable flag error!"));
				goto setErr_encrypt;
			}
		}
#endif	
#if defined(WIFI_QUICK_REINIT)
		if((old_value != intVal) || (intVal == 1))
			quick_reinit = 0;
#endif

		if ( apmib_set( MIB_WLAN_ENABLE_1X, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set 1x enable flag error!"));
			goto setErr_encrypt;
		}

		if (encrypt == ENCRYPT_WEP) {
	 		WEP_T wep;
			if ( !apmib_get( MIB_WLAN_WEP,  (void *)&wep) ) {
				strcpy(tmpBuf, ("Get MIB_WLAN_WEP MIB error!"));
				goto setErr_encrypt;
			}
			if (wep == WEP_DISABLED) {
				wep = WEP64;
				if ( apmib_set( MIB_WLAN_WEP, (void *)&wep) == 0) {
		  			strcpy(tmpBuf, ("Set WEP MIB error!"));
					goto setErr_encrypt;
				}
			}
		}
		else {
			sprintf(varName, "useMacAuth%d", wlan_id);
			strVal = get_cstream_var(postData,len,  varName, "");
			if ( !strcmp(strVal, "ON")) {
				intVal = 1;
				enableRS = 1;
			}
			else
				intVal = 0;
			if ( apmib_set( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal) == 0) {
  				strcpy(tmpBuf, ("Set MIB_WLAN_MAC_AUTH_ENABLED MIB error!"));
				goto setErr_encrypt;
			}
		}
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT	
	else if(ENCRYPT_WAPI==encrypt)
	{
		/*WAPI handle*/
		sprintf(varName, "wapiAuth%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) 
		{
			if ( !strcmp(strVal, ("eap")))
			{
				apmib_get( MIB_WLAN_MODE, (void *)&intVal);
				if (intVal!=AP_MODE && intVal!=AP_WDS_MODE) { // not AP mode
					strcpy(tmpBuf, ("Error! WAPI AS cannot be used when device is set to client mode."));
					goto setErr_encrypt;
				}
				intVal = WAPI_AUTH_AUTO;
				enableAS = 1;
			}
			else if ( !strcmp(strVal, ("psk"))) 
			{
				intVal = WAPI_AUTH_PSK;
				getPSK = 1;
			}
			else 
			{
				strcpy(tmpBuf, ("Error! Invalid wapi authentication value."));
				goto setErr_encrypt;
			}

			if ( apmib_set(MIB_WLAN_WAPI_AUTH, (void *)&intVal) == 0) 
			{
				strcpy(tmpBuf, ("Set MIB_WLAN_WAPI_AUTH failed!"));
				goto setErr_encrypt;
			}
		}
		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "wapiPskFormat%d", wlan_id);
   			strVal = get_cstream_var(postData,len,  varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_encrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_encrypt;
			}

			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);

			sprintf(varName, "wapiPskValue%d", wlan_id);
			strVal = get_cstream_var(postData,len,varName, "");
			length = strlen(strVal);

			if (oldFormat == intVal && length == oldPskLen ) {
				for (i=0; i<length; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == length)
					if(!strcmp(strVal,tmpBuf))
						goto wapi_end;
			}

			if ( apmib_set(MIB_WLAN_WAPI_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_PSK_FORMAT failed!"));
				goto setErr_encrypt;
			}

			if (intVal==1) { // hex
				if (/*len!=MAX_PSK_LEN ||*/!string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
				if(0 ==(length % 2))
				{
					length = length/2;
				}
				else
				{
					/*wapi hex key len should be even*/
					strcpy(tmpBuf, ("Error! invalid psk len."));
					goto setErr_encrypt;
				}					
				if(!apmib_set(MIB_WLAN_WAPI_PSKLEN,(void*)&length))
				{
					strcpy(tmpBuf,("Error! Set wapi key len fault"));
				}
			}
			else { // passphras
				if (length==0 || length > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
				if(!apmib_set(MIB_WLAN_WAPI_PSKLEN,(void*)&length))
				{
					strcpy(tmpBuf,("Error! Set wapi key len fault"));
				}
			}
			if ( !apmib_set(MIB_WLAN_WAPI_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_PSK error!"));
				goto setErr_encrypt;
			}
		}
	wapi_end:
		/*save AS IP*/
		if(1==enableAS)
		{ 
			int old_vwlan_idx,i;
			sprintf(varName, "wapiASIP%d", wlan_id);
			strVal = get_cstream_var(postData,len,varName, "");
			if (!strVal[0]) {
				strcpy(tmpBuf, ("No WAPI AS address!"));
				goto setErr_encrypt;
			}
			if ( !inet_aton(strVal, &inIp) ) {
				strcpy(tmpBuf, ("Invalid AS IP-address value!"));
				goto setErr_encrypt;
			}

			sprintf(varName, "wapiCertSel%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			if (!strVal[0]) {
				strcpy(tmpBuf, ("No WAPI cert selected!"));
				goto setErr_encrypt;
			}
			intVal=atoi(strVal);

			// To record old vwlan_idx
			old_vwlan_idx=vwlan_idx;
			// Set current MIB_WLAN_WAPI_ASIPADDR and MIB_WLAN_WAPI_CERT_SEL to all wlan interfaces
			// root wlan interface and virtual wlan interface
			for(i=0;i<NUM_VWLAN_INTERFACE+1;i++)
			{
				vwlan_idx=i;
				if ( !apmib_set(MIB_WLAN_WAPI_ASIPADDR, (void *)&inIp)) {
					strcpy(tmpBuf, ("Set RS IP-address error!"));
					goto setErr_encrypt;
				}	
				if ( !apmib_set(MIB_WLAN_WAPI_CERT_SEL, (void *)&intVal)) {
					strcpy(tmpBuf, ("Set WAPI cert sel error!"));
					goto setErr_encrypt;
				}	
			}
			// Back to old vwlan_idx
			vwlan_idx=old_vwlan_idx;
		}
	}
#endif
	else {
		// support nonWPA client

		sprintf(varName, "nonWpaSupp%d", wlan_id);
 		strVal = get_cstream_var(postData,len, varName, "");
		apmib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
		if(strVal[0])
		{
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		}
		if ( apmib_set( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set MIB_WLAN_ENABLE_SUPP_NONWPA mib error!"));
			goto setErr_encrypt;
		}
		if ( intVal ) {
			suppNonWPA = SUPP_NONWPA_NONE;
			sprintf(varName, "nonWpaWep%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			if ( !strcmp(strVal, "ON"))
				suppNonWPA |= SUPP_NONWPA_WEP;

			sprintf(varName, "nonWpa1x%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			if ( !strcmp(strVal, "ON")) {
				suppNonWPA |= SUPP_NONWPA_1X;
				enableRS = 1;
			}

			if ( apmib_set( MIB_WLAN_SUPP_NONWPA, (void *)&suppNonWPA) == 0) {
  				strcpy(tmpBuf, ("Set MIB_WLAN_SUPP_NONWPA mib error!"));
				goto setErr_encrypt;
			}
		}

		// WPA authentication
		sprintf(varName, "wpaAuth%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !strcmp(strVal, ("eap"))) {
				apmib_get( MIB_WLAN_MODE, (void *)&intVal);
#ifndef TLS_CLIENT
				if (intVal!=AP_MODE && intVal!=AP_WDS_MODE) { // not AP mode
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					if(intVal == CLIENT_MODE){//client mode
//						printf("%s(%d): WPA-RADIUS can be used when device is set to client mode\n",__FUNCTION__,__LINE__);//Added for test 
					}
					else{
						strcpy(tmpBuf, ("Error! WPA-RADIUS cannot be used when device is set to wds or mesh mode."));
						goto setErr_encrypt;
					}
						
#else
					strcpy(tmpBuf, ("Error! WPA-RADIUS cannot be used when device is set to client mode."));
					goto setErr_encrypt;
#endif
				}
#endif
				intVal = WPA_AUTH_AUTO;
				enableRS = 1;
			}
			else if ( !strcmp(strVal, ("psk"))) {
				intVal = WPA_AUTH_PSK;
				getPSK = 1;

			}
			else {
				strcpy(tmpBuf, ("Error! Invalid wpa authentication value."));
				goto setErr_encrypt;
			}
			if ( apmib_set(MIB_WLAN_WPA_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_AUTH failed!"));
				goto setErr_encrypt;
			}
		}

		// cipher suite		
		// sc_yang write the ciphersuite according to  encrypt for wpa
		// wpa mixed mode is not implemented yet.
		
// get cipher suite from user setting, for wpa-aes -------------------		
#if 0				
		intVal = 0 ;
		if( (encrypt ==  ENCRYPT_WPA) || (encrypt == ENCRYPT_WPA2_MIXED) )
			intVal =   WPA_CIPHER_TKIP ;
		if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;
		}
		//set wpa2UniCipher  for wpa2
		// wpa2 mixed mode is not implemented yet.
		intVal = 0 ;
		if( (encrypt ==  ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED) )
			intVal =   WPA_CIPHER_AES ;
		if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_UNICIPHER failed!"));
				goto setErr_encrypt;
		}
#endif	
		//if ((encrypt == ENCRYPT_WPA) || (encrypt == ENCRYPT_WPA2_MIXED)) 
		{
			sprintf(varName, "ciphersuite%d", wlan_id);
			strVal = get_cstream_var(postData,len,varName, "");	 	
			if (strVal[0]) {
				intVal = 0;				
				if ( strstr(strVal, ("tkip"))) 
					intVal |= WPA_CIPHER_TKIP;
				if ( strstr(strVal, ("aes"))) 
					intVal |= WPA_CIPHER_AES;
				if (intVal == 0) {
					strcpy(tmpBuf, ("Invalid value of cipher suite!"));
					goto setErr_encrypt;
				}
			}
			else{
					intVal = WPA_CIPHER_TKIP;	
			}

			// check if both TKIP and AES cipher are selected in client mode
			/*apmib_get(MIB_WLAN_MODE, (void *)&val);
			if (val == CLIENT_MODE) {
				apmib_get(MIB_WLAN_NETWORK_TYPE, &val);
				if (val == INFRASTRUCTURE && intVal == WPA_CIPHER_MIXED) {
					strcpy(tmpBuf, ("Error! Can't set cipher to TKIP + AES when device is set to client mode."));
					goto setErr_encrypt;							
				}
			}	// david+2006-1-11*/
					
			if ( apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;							
			}				
		}		
		//if ((encrypt == ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED)) 
		{
			sprintf(varName, "wpa2ciphersuite%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");	 	
			if (strVal[0]) {
				intVal = 0;				
				if ( strstr(strVal, ("tkip"))) 
					intVal |= WPA_CIPHER_TKIP;
				if ( strstr(strVal, ("aes"))) 
					intVal |= WPA_CIPHER_AES;
				if (intVal == 0) {
					strcpy(tmpBuf, ("Invalid value of wpa2 cipher suite!"));
					goto setErr_encrypt;
				}
			}
			else
				intVal = WPA_CIPHER_AES;			

			// check if both TKIP and AES cipher are selected in client mode
			/*apmib_get(MIB_WLAN_MODE, (void *)&val);
			if (val == CLIENT_MODE) {
				apmib_get(MIB_WLAN_NETWORK_TYPE, &val);
				if (val == INFRASTRUCTURE && intVal == WPA_CIPHER_MIXED) {
					strcpy(tmpBuf, ("Error! Can't set cipher to TKIP + AES when device is set to client mode."));
					goto setErr_encrypt;							
				}
			}	// david+2006-1-11*/
				
			if ( apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA2_CIPHER_SUITE failed!"));
				goto setErr_encrypt;							
			}
		}
//-------------------------------------------------- david, 2005-8-03	
	
		if( ((encrypt ==  ENCRYPT_WPA2) || (encrypt == ENCRYPT_WPA2_MIXED)) &&
		    enableRS == 1){
			sprintf(varName, "preAuth%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			if ( !strcmp(strVal, "ON"))
				intVal = 1 ;
			else
				intVal = 0 ;
			if ( apmib_set(MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_CIPHER_SUITE failed!"));
				goto setErr_encrypt;
			}					
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_id);
   			strVal = get_cstream_var(postData,len, varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_encrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_encrypt;
			}

			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_WLAN_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_WLAN_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);

			sprintf(varName, "pskValue%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			length = strlen(strVal);

			if (oldFormat == intVal && length == oldPskLen ) {
				for (i=0; i<length; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == length)
					if(!strcmp(strVal,tmpBuf))
						goto rekey_time;
			}

			if ( apmib_set(MIB_WLAN_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_PSK_FORMAT failed!"));
				goto setErr_encrypt;
			}

			if (intVal==1) { // hex
				if (length!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
			}
			else { // passphras
				if (length==0 || length > (MAX_PSK_LEN-1)|| length < MIN_PSK_LEN ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_encrypt;
				}
			}
			if ( !apmib_set(MIB_WLAN_WPA_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_PSK error!"));
				goto setErr_encrypt;
			}
		}
rekey_time:
		// group key rekey time
		reKeyTime = 0;
		sprintf(varName, "groupKeyTimeDay%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey day."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*86400;
		}
		sprintf(varName, "groupKeyTimeHr%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey hr."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*3600;
		}
		sprintf(varName, "groupKeyTimeMin%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey min."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal*60;
		}

		sprintf(varName, "groupKeyTimeSec%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of rekey sec."));
				goto setErr_encrypt;
			}
			reKeyTime += intVal;
		}
		if (reKeyTime) {
			if ( !apmib_set(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&reKeyTime)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WPA_GROUP_REKEY_TIME error!"));
				goto setErr_encrypt;
			}
		}
	}
#if 0 //defined(WIFI_QUICK_REINIT)
			if(old_value != enableRS)
				quick_reinit = 0;
#endif
	apmib_set( MIB_WLAN_ENABLE_1X, (void *)&enableRS);			
	if (enableRS == 1) { // if 1x enabled, get RADIUS server info
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
		apmib_get( MIB_WLAN_MODE, (void *)&wlan_mode);
		if (wlan_mode == CLIENT_MODE) { // wlan client mode
			wlanIdx_5G=whichWlanIfIs(PHYBAND_5G);
			wlanIdx_2G=whichWlanIfIs(PHYBAND_2G);
			if(wlan_idx==wlanIdx_5G){
				rsBandSel=PHYBAND_5G;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			else if(wlan_idx==wlanIdx_2G){
				rsBandSel=PHYBAND_2G;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			else{
				rsBandSel=PHYBAND_OFF;
				if ( !apmib_set(MIB_WLAN_RS_BAND_SEL, (void *)&rsBandSel)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_RS_BAND_SEL error!"));
					goto setErr_encrypt;
				}
			}
			
			sprintf(varName, "eapType%d", wlan_id);
			strVal = get_cstream_var(postData,len, varName, "");
			if (strVal[0]) {
				if ( !string_to_dec(strVal, &intVal) ) {
					strcpy(tmpBuf, ("Invalid 802.1x EAP type value!"));
					goto setErr_encrypt;
				}
				if ( !apmib_set(MIB_WLAN_EAP_TYPE, (void *)&intVal)) {
					strcpy(tmpBuf, ("Set MIB_WLAN_EAP_TYPE error!"));
					goto setErr_encrypt;
				}
			}
			else{
				strcpy(tmpBuf, ("No 802.1x EAP type!"));
				goto setErr_encrypt;
			}

			if(intVal == EAP_MD5){
				sprintf(varName, "eapUserId%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
						strcpy(tmpBuf, ("EAP user ID too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
					goto setErr_encrypt;
				}
				
				sprintf(varName, "radiusUserName%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_NAME_LEN){
						strcpy(tmpBuf, ("RADIUS user name too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_NAME, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_NAME error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x RADIUS User Name!"));
					goto setErr_encrypt;
				}

				sprintf(varName, "radiusUserPass%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_PASS_LEN){
						strcpy(tmpBuf, ("RADIUS user password too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_PASSWD error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x RADIUS User Password!"));
					goto setErr_encrypt;
				}
			}
			else if(intVal == EAP_TLS){
				sprintf(varName, "eapUserId%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
						strcpy(tmpBuf, ("EAP user ID too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
					goto setErr_encrypt;
				}
				
				sprintf(varName, "radiusUserCertPass%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if(strlen(strVal)>MAX_RS_USER_CERT_PASS_LEN){
						strcpy(tmpBuf, ("RADIUS user cert password too long!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_CERT_PASSWD error!"));
						goto setErr_encrypt;
					}
				}
				else{
					if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
						strcpy(tmpBuf, ("Clear MIB_WLAN_RS_USER_CERT_PASSWD error!"));
						goto setErr_encrypt;
					}
					//strcpy(tmpBuf, ("No 802.1x RADIUS user cert password!"));
					//goto setErr_encrypt;
				}

				if(rsBandSel == PHYBAND_5G){
					if(isFileExist(RS_USER_CERT_5G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 5g user cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
					
					if(isFileExist(RS_ROOT_CERT_5G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 5g root cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
				}
				else if(rsBandSel == PHYBAND_2G){
					if(isFileExist(RS_USER_CERT_2G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 2g user cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
					
					if(isFileExist(RS_ROOT_CERT_2G) != 1){
						strcpy(tmpBuf, ("No 802.1x RADIUS 2g root cert!\nPlease upload it."));
						goto setErr_encrypt;
					}
				}
			}
			else if(intVal == EAP_PEAP){
				sprintf(varName, "eapInsideType%d", wlan_id);
				strVal = get_cstream_var(postData,len, varName, "");
				if (strVal[0]) {
					if ( !string_to_dec(strVal, &intVal2) ) {
						strcpy(tmpBuf, ("Invalid 802.1x inside tunnel type value!"));
						goto setErr_encrypt;
					}
					if ( !apmib_set(MIB_WLAN_EAP_INSIDE_TYPE, (void *)&intVal2)) {
						strcpy(tmpBuf, ("Set MIB_WLAN_EAP_INSIDE_TYPE error!"));
						goto setErr_encrypt;
					}
				}
				else{
					strcpy(tmpBuf, ("No 802.1x inside tunnel type!"));
					goto setErr_encrypt;
				}

				if(intVal2 == INSIDE_MSCHAPV2){
					sprintf(varName, "eapUserId%d", wlan_id);
					strVal = get_cstream_var(postData,len,varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_EAP_USER_ID_LEN){
							strcpy(tmpBuf, ("EAP user ID too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_EAP_USER_ID, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_EAP_USER_ID error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x EAP User ID!"));
						goto setErr_encrypt;
					}
					
					sprintf(varName, "radiusUserName%d", wlan_id);
					strVal = get_cstream_var(postData,len, varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_RS_USER_NAME_LEN){
							strcpy(tmpBuf, ("RADIUS user name too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_RS_USER_NAME, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_NAME error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x RADIUS User Name!"));
						goto setErr_encrypt;
					}

					sprintf(varName, "radiusUserPass%d", wlan_id);
					strVal = get_cstream_var(postData,len, varName, "");
					if (strVal[0]) {
						if(strlen(strVal)>MAX_RS_USER_PASS_LEN){
							strcpy(tmpBuf, ("RADIUS user password too long!"));
							goto setErr_encrypt;
						}
						if ( !apmib_set(MIB_WLAN_RS_USER_PASSWD, (void *)strVal)) {
							strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_PASSWD error!"));
							goto setErr_encrypt;
						}
					}
					else{
						strcpy(tmpBuf, ("No 802.1x RADIUS User Password!"));
						goto setErr_encrypt;
					}

//					if(isFileExist(RS_USER_CERT) == 1){
						sprintf(varName, "radiusUserCertPass%d", wlan_id);
						strVal = get_cstream_var(postData,len,varName, "");
						if (strVal[0]) {
							if(strlen(strVal)>MAX_RS_USER_CERT_PASS_LEN){
								strcpy(tmpBuf, ("RADIUS user cert password too long!"));
								goto setErr_encrypt;
							}
							if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
								strcpy(tmpBuf, ("Set MIB_WLAN_RS_USER_CERT_PASSWD error!"));
								goto setErr_encrypt;
							}
						}
						else{
							if ( !apmib_set(MIB_WLAN_RS_USER_CERT_PASSWD, (void *)strVal)) {
								strcpy(tmpBuf, ("[1] Clear MIB_WLAN_RS_USER_CERT_PASSWD error!"));
								goto setErr_encrypt;
							}
							//strcpy(tmpBuf, ("No 802.1x RADIUS user cert password!"));
							//goto setErr_encrypt;
						}
//					}
				}
				else{
					strcpy(tmpBuf, ("802.1x inside tunnel type not support!"));
					goto setErr_encrypt;
				}
			}
			else{
				strcpy(tmpBuf, ("802.1x EAP type not support!"));
				goto setErr_encrypt;
			}
		}
		else
#endif
		{
		sprintf(varName, "radiusPort%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of RS port number."));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_PORT, (void *)&intVal)) {
			strcpy(tmpBuf, ("Set RS port error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "radiusIP%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No RS IP address!"));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, ("Invalid RS IP-address value!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, ("Set RS IP-address error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "radiusPass%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strlen(strVal) > (MAX_RS_PASS_LEN -1) ) {
			strcpy(tmpBuf, ("RS password length too long!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, ("Set RS password error!"));
			goto setErr_encrypt;
		}

		sprintf(varName, "radiusRetry%d", wlan_id);
		strVal = get_cstream_var(postData,len,varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid RS retry value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_RS_MAXRETRY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_RS_MAXRETRY error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "radiusTime%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid RS time value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_RS_INTERVAL_TIME, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_RS_INTERVAL_TIME error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "useAccount%d", wlan_id);
		strVal = get_cstream_var(postData,len,varName, "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal) == 0) {
  			strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		if (intVal == 0)
			goto get_wepkey;

		sprintf(varName, "accountPort%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No account RS port number!"));
			goto setErr_encrypt;
		}
		if (!string_to_dec(strVal, &intVal) || intVal<=0 || intVal>65535) {
			strcpy(tmpBuf, ("Error! Invalid value of account RS port number."));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal)) {
			strcpy(tmpBuf, ("Set account RS port error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountIP%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (!strVal[0]) {
			strcpy(tmpBuf, ("No account RS IP address!"));
			goto setErr_encrypt;
		}
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, ("Invalid account RS IP-address value!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_IP, (void *)&inIp)) {
			strcpy(tmpBuf, ("Set account RS IP-address error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountPass%d", wlan_id);
		strVal = get_cstream_var(postData,len,varName, "");
		if (strlen(strVal) > (MAX_RS_PASS_LEN -1) ) {
			strcpy(tmpBuf, ("Account RS password length too long!"));
			goto setErr_encrypt;
		}
		if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, ("Set account RS password error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountRetry%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid account RS retry value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_MAXRETRY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_MAXRETRY error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "accountTime%d", wlan_id);
		strVal = get_cstream_var(postData,len,varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Invalid account RS time value!"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME error!"));
				goto setErr_encrypt;
			}
		}
		sprintf(varName, "accountUpdateEnabled%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if ( !strcmp(strVal, "ON"))
			intVal = 1;
		else
			intVal = 0;
		if ( apmib_set( MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED, (void *)&intVal) == 0) {
			strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED mib error!"));
			goto setErr_encrypt;
		}
		sprintf(varName, "accountUpdateTime%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !string_to_dec(strVal, &intVal) ) {
				strcpy(tmpBuf, ("Error! Invalid value of update time"));
				goto setErr_encrypt;
			}
			if ( !apmib_set(MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY, (void *)&intVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY mib error!"));
				goto setErr_encrypt;
			}
		}

get_wepkey:
		// get 802.1x WEP key length
		sprintf(varName, "wepKeyLen%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( !strcmp(strVal, ("wep64")))
				intVal = WEP64;
			else if ( !strcmp(strVal, ("wep128")))
				intVal = WEP128;
			else {
				strcpy(tmpBuf, ("Error! Invalid wepkeylen value."));
				goto setErr_encrypt;
			}
			if ( apmib_set(MIB_WLAN_WEP, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_WEP failed!"));
				goto setErr_encrypt;
			}
		}
	}
	}

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		val = 0;
		if (strVal[0])
			val = atoi(strVal);
		update_wps_configured(val);
	}
#endif

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	if (apmib_get_vwlanidx() == NUM_VWLAN_INTERFACE)
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		strVal = get_cstream_var(postData,len, varName, "");
		val = 0;
		if (strVal[0])
			val = atoi(strVal);
		update_wps_configured(val);
	}
#endif
#if defined(WIFI_QUICK_REINIT)
	return quick_reinit;
#else
	return 0 ;
#endif
setErr_encrypt:
	return -1 ;		
}
#if defined(WLAN_PROFILE)
int wlanProfileEncryptHandler(char *postData, int len, char *tmpBuf)
{
	char varName[20];
	char *strEncrypt, *strVal;
	int ssid_idx;
	int profile_num_id,	profile_tbl_id, profile_mod_id;
	WLAN_PROFILE_T entry;
	WLAN_PROFILE_T target[2];
	ENCRYPT_T encrypt;
	int wlan_idx=apmib_get_wlanidx();

//displayPostDate(wp->post_data);
	memset(target, 0x00, sizeof(WLAN_PROFILE_T)*2);
	strVal = get_cstream_var(postData,len, "SSID_Setting", "");
		
	if (strVal[0])
		ssid_idx = atoi(strVal);

#if defined(UNIVERSAL_REPEATER) 
	if(ssid_idx < (1+NUM_VWLAN+1) || ssid_idx>=(1+NUM_VWLAN+1+MAX_WLAN_PROFILE_NUM))
#else
	if(ssid_idx < (1+NUM_VWLAN) || ssid_idx>=(1+NUM_VWLAN+MAX_WLAN_PROFILE_NUM))
#endif		
	{
		strcpy(tmpBuf, ("ssid_idx is invalid!!"));
		goto setErr_wlan;
	}

	
	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_mod_id = MIB_PROFILE_MOD1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_mod_id = MIB_PROFILE_MOD2;
	}

#if defined(UNIVERSAL_REPEATER) 
	ssid_idx -= (1+NUM_VWLAN+1);
#else
	ssid_idx -= (1+NUM_VWLAN);
#endif
	ssid_idx++;

//printf("\r\n ssid_idx=[%d],__[%s-%u]\r\n",ssid_idx,__FILE__,__LINE__);

	*((char *)&entry) = (char)ssid_idx;
	if ( !apmib_get(profile_tbl_id, (void *)&entry)) {
		strcpy(tmpBuf, ("Get table entry error!"));
		goto setErr_wlan;
	}

	memcpy(&target[0], &entry, sizeof(WLAN_PROFILE_T));

	sprintf(varName, "method%d", wlan_idx);
	strEncrypt = get_cstream_var(postData,len, varName, "");
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';

//printf("\r\n encrypt[%d],__[%s-%u]\r\n",encrypt,__FILE__,__LINE__);

	if (encrypt==ENCRYPT_WEP) 
	{
		char *strWep, *strAuth, *strFormat, *wepKey;
		int keyLen;
		char key[30];
		
		sprintf(varName, "length%d", wlan_idx);
		strWep = get_cstream_var(postData,len, varName, "");
		if(strWep[0])
		{
			entry.encryption = atoi(strWep);
		}

		sprintf(varName, "format%d", wlan_idx);
		strFormat = get_cstream_var(postData,len, varName, "");
		if(strFormat[0])
		{
			entry.wepKeyType = atoi(strFormat)-1;
		}
		
		strAuth = get_cstream_var(postData,len, ("authType"), "");
		if (strAuth[0]) { // new UI
			if (!strcmp(strAuth, ("open")))
				entry.auth = AUTH_OPEN;
			else if ( !strcmp(strAuth, ("shared")))
				entry.auth = AUTH_SHARED;
			else 
				entry.auth = AUTH_BOTH;			
		}

		sprintf(varName, "key%d", wlan_idx);
		wepKey = get_cstream_var(postData,len, varName, "");
		if  (wepKey[0]) {

			if ( !isAllStar(wepKey) ) {

				if (entry.encryption == WEP64) {
					if (entry.wepKeyType==0)
						keyLen = WEP64_KEY_LEN;
					else
						keyLen = WEP64_KEY_LEN*2;
				}
				else 
				{
					if (entry.wepKeyType==0)
						keyLen = WEP128_KEY_LEN;
					else
						keyLen = WEP128_KEY_LEN*2;
				}
		
				if (entry.wepKeyType == 0) // ascii
					strcpy(key, wepKey);
				else // hex
				{ 
					if ( !string_to_hex(wepKey, key, keyLen)) {
		   				strcpy(tmpBuf, ("Invalid wep-key value!"));
						goto setErr_wlan;
					}
				}
				if (entry.encryption == WEP64){
					strncpy(entry.wepKey1, key, 10);
					strncpy(entry.wepKey2, key, 10);
					strncpy(entry.wepKey3, key, 10);
					strncpy(entry.wepKey4, key, 10);
				}else{
					strncpy(entry.wepKey1, key, 26);
					strncpy(entry.wepKey2, key, 26);
					strncpy(entry.wepKey3, key, 26);
					strncpy(entry.wepKey4, key, 26);
				}			
			}
		}
		
	}
	else if (encrypt > ENCRYPT_WEP) 
	{
		char *strWpaAuth, *strCipherSuite, *strPskFormat, *strPskValue;
		int cipherSuite, pskFormat;
		int getPSK;
		// WPA authentication

		if(encrypt == ENCRYPT_WPA)
			entry.encryption = 3; //wpa
		else
			entry.encryption = 4; //wpa2
		
		sprintf(varName, "wpaAuth%d", wlan_idx);
		strWpaAuth = get_cstream_var(postData,len, varName, "");
		if (strWpaAuth[0]) 
		{
			if ( !strcmp(strWpaAuth, ("eap"))) {
				strcpy(tmpBuf, ("Invalid wpaAuth value!"));
				goto setErr_wlan;
			}
			else if ( !strcmp(strWpaAuth, ("psk"))) {
				getPSK = 1;
			}
			else {
				strcpy(tmpBuf, ("Error! Invalid wpa authentication value."));
				goto setErr_wlan;
			}
		}

		sprintf(varName, "ciphersuite%d", wlan_idx);
		strCipherSuite = get_cstream_var(postData,len, varName, "");	 	
		if (strCipherSuite[0]) {
			cipherSuite = 0;				
			if ( strstr(strCipherSuite, ("tkip"))) 
				cipherSuite |= WPA_CIPHER_TKIP;
			if ( strstr(strCipherSuite, ("aes"))) 
				cipherSuite |= WPA_CIPHER_AES;
			if (cipherSuite == 0 || cipherSuite == WPA_CIPHER_MIXED) //check if both TKIP and AES cipher are selected in client mode
			{
				strcpy(tmpBuf, ("Invalid value of cipher suite!"));
				goto setErr_wlan;
			}

			if(cipherSuite == WPA_CIPHER_TKIP)
				entry.wpa_cipher = 2;
			else
				entry.wpa_cipher = 8;
			
		}		
		{
			sprintf(varName, "wpa2ciphersuite%d", wlan_idx);
			strCipherSuite = get_cstream_var(postData,len, varName, "");	 	
			if (strCipherSuite[0]) {
				cipherSuite = 0;				
				if ( strstr(strCipherSuite, ("tkip"))) 
					cipherSuite |= WPA_CIPHER_TKIP;
				if ( strstr(strCipherSuite, ("aes"))) 
					cipherSuite |= WPA_CIPHER_AES;
				if (cipherSuite == 0 || cipherSuite == WPA_CIPHER_MIXED) //check if both TKIP and AES cipher are selected in client mode
				{
					strcpy(tmpBuf, ("Invalid value of cipher suite!"));
					goto setErr_wlan;
				}

				if(cipherSuite == WPA_CIPHER_TKIP)
					entry.wpa_cipher = 2;
				else
					entry.wpa_cipher = 8;
				
			}
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_idx);
   			strPskFormat = get_cstream_var(postData,len, varName, "");
			if (!strPskFormat[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_wlan;
			}
			pskFormat = strPskFormat[0] - '0';
			if (pskFormat != 0 && pskFormat != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_wlan;
			}

			// remember current psk format and length to compare to default case "****"
			sprintf(varName, "pskValue%d", wlan_idx);
			strPskValue = get_cstream_var(postData,len, varName, "");

			entry.wpaPSKFormat= pskFormat;

			if (pskFormat==1) { // hex
				if (strlen(strPskValue)!=MAX_PSK_LEN || !string_to_hex(strPskValue, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_wlan;
				}
			}
			else { // passphras
				if (strlen(strPskValue)==0 || strlen(strPskValue) > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_wlan;
				}
			}
			strcpy(entry.wpaPSK, strPskValue);
		
		}

	}
	else
	{
		char oldSsid[32];

		strcpy(oldSsid, entry.ssid);
		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		strcpy(entry.ssid, oldSsid);
		entry.wpa_cipher = 8; //WPA_CIPHER_AES
		
	}

	memcpy(&target[1], &entry, sizeof(WLAN_PROFILE_T));

	if ( !apmib_set(profile_mod_id, (void *)&target)) {
		strcpy(tmpBuf, ("Modify table entry error!"));
		goto setErr_wlan;
	}

	return 0 ;
setErr_wlan:
	
	return -1 ;	
}
#endif //#if defined(WLAN_PROFILE)

int wepHandler(char *postData, int len, char *tmpBuf, int wlan_id)
{
   	char  *wepKey;
   	char *strKeyLen, *strFormat, *strAuth, /* *strKeyId, */ *strEnabled;
	char key[30];
	int enabled, keyLen, ret, i;
	WEP_T wep;
	ENCRYPT_T encrypt=ENCRYPT_WEP;
	char varName[32];
	int wlanMode, rptEnabled;
	int authType;

//printf("\r\n wlan_idx=[%d],vwlan_idx=[%d],__[%s-%u]\r\n",wlan_idx,vwlan_idx,__FILE__,__LINE__);	
//displayPostDate(wp->post_data);

#ifdef WIFI_SIMPLE_CONFIG
#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		memset(&wps_config_info, 0, sizeof(struct wps_config_info_struct));
		wps_config_info.caller_id = CALLED_FROM_WEPHANDLER;
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&wps_config_info.auth);
		apmib_get(MIB_WLAN_WEP, (void *)&wps_config_info.wep_enc);
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wps_config_info.KeyId);
		apmib_get(MIB_WLAN_WEP64_KEY1, (void *)wps_config_info.wep64Key1);
		apmib_get(MIB_WLAN_WEP64_KEY2, (void *)wps_config_info.wep64Key2);
		apmib_get(MIB_WLAN_WEP64_KEY3, (void *)wps_config_info.wep64Key3);
		apmib_get(MIB_WLAN_WEP64_KEY4, (void *)wps_config_info.wep64Key4);
		apmib_get(MIB_WLAN_WEP128_KEY1, (void *)wps_config_info.wep128Key1);
		apmib_get(MIB_WLAN_WEP128_KEY2, (void *)wps_config_info.wep128Key2);
		apmib_get(MIB_WLAN_WEP128_KEY3, (void *)wps_config_info.wep128Key3);
		apmib_get(MIB_WLAN_WEP128_KEY4, (void *)wps_config_info.wep128Key4);
	}
#endif

	sprintf(varName, "wepEnabled%d", wlan_id);
	strEnabled = get_cstream_var(postData,len, varName, "");
	if ( !strcmp(strEnabled, "ON"))
		enabled = 1;
	else
		enabled = 0;

	if ( enabled ) {
		sprintf(varName, "length%d", wlan_id);
		strKeyLen =get_cstream_var(postData,len, varName, "");
		if (!strKeyLen[0]) {
 			strcpy(tmpBuf, ("Key length must exist!"));
			goto setErr_wep;
		}
		if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
 			strcpy(tmpBuf, ("Invalid key length value!"));
			goto setErr_wep;
		}
		if (strKeyLen[0] == '1')
			wep = WEP64;
		else
			wep = WEP128;
	}
	else
		wep = WEP_DISABLED;

	if ( apmib_set( MIB_WLAN_WEP, (void *)&wep) == 0) {
  		strcpy(tmpBuf, ("Set WEP MIB error!"));
		goto setErr_wep;
	}

	if (wep == WEP_DISABLED)
		encrypt = ENCRYPT_DISABLED;

	if (apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt) == 0) {
		strcpy(tmpBuf, ("Set MIB_WLAN_ENCRYPT mib error!"));
		goto setErr_wep;
	}

	if (wep == WEP_DISABLED)
		return 0 ;


	sprintf(varName, "authType%d", wlan_id);
	strAuth = get_cstream_var(postData,len, varName, "");
	if (strAuth[0]) { // new UI
		if (!strcmp(strAuth, ("open")))
			authType = AUTH_OPEN;
		else if ( !strcmp(strAuth, ("shared")))
			authType = AUTH_SHARED;
		else 
			authType = AUTH_BOTH;
		apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType);
	}
	else {
		authType = AUTH_BOTH;
		apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType);
	}
				
	sprintf(varName, "format%d", wlan_id);
	strFormat = get_cstream_var(postData,len, varName, "");
	if (!strFormat[0]) {
 		strcpy(tmpBuf, ("Key type must exist!"));
		goto setErr_wep;
	}

	if (strFormat[0]!='1' && strFormat[0]!='2') {
		strcpy(tmpBuf, ("Invalid key type value!"));
		goto setErr_wep;
	}

	i = strFormat[0] - '0' - 1;
	if ( apmib_set( MIB_WLAN_WEP_KEY_TYPE, (void *)&i) == 0) {
  		strcpy(tmpBuf, ("Set WEP key type error!"));
		goto setErr_wep;
	}

	if (wep == WEP64) {
		if (strFormat[0]=='1')
			keyLen = WEP64_KEY_LEN;
		else
			keyLen = WEP64_KEY_LEN*2;
	}
	else {
		if (strFormat[0]=='1')
			keyLen = WEP128_KEY_LEN;
		else
			keyLen = WEP128_KEY_LEN*2;
	}
		sprintf(varName, "key%d", wlan_id);
	wepKey = get_cstream_var(postData,len, varName, "");
	if  (wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			sprintf(tmpBuf, ("Invalid key length! Expect length is %d"), keyLen);
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1') // ascii
				strcpy(key, wepKey);
			else { // hex
				if ( !string_to_hex(wepKey, key, keyLen)) {
	   				strcpy(tmpBuf, ("Invalid wep-key value!"));
					goto setErr_wep;
				}
			}
			if (wep == WEP64){
				ret=apmib_set(MIB_WLAN_WEP64_KEY1, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY2, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY3, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP64_KEY4, (void *)key);
			}else{
				ret=apmib_set(MIB_WLAN_WEP128_KEY1, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY2, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY3, (void *)key);
				ret=apmib_set(MIB_WLAN_WEP128_KEY4, (void *)key);
			}
			if (!ret) {
	 			strcpy(tmpBuf, ("Set wep-key error!"));
				goto setErr_wep;
			}
		}
	}

	
	
#ifdef WIFI_SIMPLE_CONFIG
	#ifdef MBSSID
	if (apmib_get_vwlanidx() == 0)
#endif
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		wepKey = get_cstream_var(postData,len, varName, "");
		ret = 0;
		if (wepKey[0])
			ret = atoi(wepKey);
		update_wps_configured(ret);
	}
#endif

#if defined(UNIVERSAL_REPEATER) && defined(CONFIG_REPEATER_WPS_SUPPORT)
	if (apmib_get_vwlanidx() == NUM_VWLAN_INTERFACE)
	{
		sprintf(varName, "wps_clear_configure_by_reg%d", wlan_id);
		wepKey = get_cstream_var(postData,len, varName, "");
		ret = 0;
		if (wepKey[0])
			ret = atoi(wepKey);
		update_wps_configured(ret);
	}
#endif

	return 0 ;
setErr_wep:
	return -1 ;	
}

void formWep(char * postData,int len)
{
	char *submitUrl;
	char tmpBuf[100];

	if(wepHandler(postData,len, tmpBuf, apmib_get_wlanidx()) < 0 )
		goto setErr_end ;

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
	//run_init_script("bridge");
#endif

#if defined(WIFI_QUICK_REINIT)
	return;
#else
	submitUrl = get_cstream_var(postData, len,("submit-url"), "");   // hidden page
	OK_MSG(submitUrl);
	return;
#endif
setErr_end:
	ERR_MSG(tmpBuf);
}

void formWlEncrypt(char *postData, int len)
{
	char *submitUrl=NULL;
	char tmpBuf[100]={0};
	char urlbuf[60]={0};
	int ret=0;
#if defined(WIFI_QUICK_REINIT)
	int old_1x_value=0, new_1x_value=0, quick_reinit=1;
#endif
//displayPostDate(wp->post_data);	
#ifdef MBSSID	
	char *strEncrypt, *strVal, *strVal1;
	char varName[40];
	int ssid_idx, ssid_idx2, old_idx=-1;

   	strVal1 = get_cstream_var(postData,len, "wlan_ssid_id", "");
   	strVal = get_cstream_var(postData,len, "SSID_Setting", "");
	submitUrl = get_cstream_var(postData, len,("submit-url"), "");	 // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
	if (strVal[0]) {
		ssid_idx2 = ssid_idx = atoi(strVal);

#if defined(CONFIG_RTL_ULINKER)
	if(ssid_idx == 5) //vxd
		ssid_idx = NUM_VWLAN_INTERFACE;
#endif
		
	mssid_idx = atoi(strVal1); // selected index from UI
	old_idx = apmib_get_vwlanidx();
	apmib_set_vwlanidx(ssid_idx);
#if defined(WLAN_PROFILE)
//printf("\r\n ssid_idx=[%d],__[%s-%u]\r\n",ssid_idx,__FILE__,__LINE__);

#if defined(UNIVERSAL_REPEATER) 
	if (ssid_idx2 >= (1+NUM_VWLAN+1))
#else
	if (ssid_idx2 >= (1+NUM_VWLAN))
#endif
	{
		if(wlanProfileEncryptHandler(postData,len, tmpBuf) == 0)
		{
			goto setOK;

		}
		else
		{
			goto setErr_end ;
		}
	}
	else
#endif //#if defined(WLAN_PROFILE)		
	{
		if (ssid_idx > NUM_VWLAN_INTERFACE) {			
			printf("Invald ssid_id!\n");
			return;
		}			
	}


		
	
		sprintf(varName, "method%d", apmib_get_wlanidx());
	   	strEncrypt = get_cstream_var(postData,len, varName, "");
		ENCRYPT_T encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
		
#if defined(WIFI_QUICK_REINIT)
		apmib_get(MIB_WLAN_ENABLE_1X, (void *)&old_1x_value);
#endif

		if (encrypt==ENCRYPT_WEP) {
			sprintf(varName, "authType%d", apmib_get_wlanidx());          
			char *strAuth = get_cstream_var(postData,len, varName, "");
			AUTH_TYPE_T authType;
			if (strAuth[0]) { // new UI
				if (!strcmp(strAuth, ("open")))
					authType = AUTH_OPEN;
				else if ( !strcmp(strAuth, ("shared")))
					authType = AUTH_SHARED;
				else 
					authType = AUTH_BOTH;
				apmib_set(MIB_WLAN_AUTH_TYPE, (void *)&authType);

				sprintf(varName, "use1x%d", apmib_get_wlanidx());
				strVal = get_cstream_var(postData,len, varName, "");	
				if (strVal[0] && strcmp(strVal, "ON")) {
					int intVal = 0;
					
					apmib_set( MIB_WLAN_ENABLE_1X, (void *)&intVal);
					formWep(postData,len);
					apmib_set_vwlanidx(old_idx);	
#ifdef HAVE_SYSTEM_REINIT
					{
						wait_redirect("Apply Changes",5,urlbuf);
						sleep(1);
						kick_reinit_m(SYS_WIFI_M|SYS_WIFI_APP_M);
					}
#elif defined(WIFI_QUICK_REINIT)
					if(MUST_REBOOT != 1)
					{
						if(old_1x_value !=0)
						{
							kick_event(RESET_EVENT);
						}
						else
						{
							kick_event(WLAN_BRIDGE_EVENT);
						}
						
						send_redirect_perm(submitUrl);
					}
					else
						OK_MSG(submitUrl);
#else
						OK_MSG(submitUrl);
#endif

					save_cs_to_file();

					return;	
				}
			}
		}
	}
	else
			mssid_idx = 0;
#endif // MBSSID

	ret = wpaHandler(postData,len, tmpBuf, apmib_get_wlanidx());
	if(ret < 0) {
#ifdef MBSSID
		if (old_idx >= 0)
			apmib_set_vwlanidx(old_idx);	
#endif		
		goto setErr_end ;
	}

setOK:
	
	apmib_update_web(CURRENT_SETTING);

	
#if defined(WIFI_QUICK_REINIT)
	apmib_get(MIB_WLAN_ENABLE_1X, (void *)&new_1x_value);
#endif

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif
	//submitUrl = get_cstream_var(postData,len, ("submit-url"), "");   // hidden page
	//sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes",5,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M|SYS_WIFI_APP_M);
	}
#elif defined(WIFI_QUICK_REINIT)
	if(MUST_REBOOT != 1)
	{
		if((old_1x_value != new_1x_value) || (new_1x_value == 1))
		{
			//use 1x setting is changed
			kick_event(RESET_EVENT);
		}
		else 
		{
			kick_event(WLAN_BRIDGE_EVENT);
		}
		
		send_redirect_perm(submitUrl);
	}
	else
		OK_MSG(submitUrl);
#else
	OK_MSG(submitUrl);
#endif



#ifdef MBSSID
	if (old_idx >= 0)
		apmib_set_vwlanidx (old_idx);	
#endif

	save_cs_to_file();

	return;

setErr_end:
	ERR_MSG(tmpBuf);
}

/******************************************************************/
void formWlAc(char *postData, int len)
{
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[100];
	char urlbuf[60];
	int entryNum, i, enabled;
	MACFILTER_T macEntry, macEntrytmp;
	int j = 0;
	
	strAddMac =get_cstream_var(postData,len, ("addFilterMac"), "");
	strDelMac = get_cstream_var(postData,len, ("deleteSelFilterMac"), "");
	strDelAllMac = get_cstream_var(postData,len, ("deleteAllFilterMac"), "");
	strEnabled = get_cstream_var(postData,len, ("wlanAcEnabled"), "");
	submitUrl = get_cstream_var(postData,len, ("submit-url"), "");   // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());

	if (strAddMac[0]) {
		/*if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0; */ //by sc_yang
		 enabled = strEnabled[0] - '0';
		if ( apmib_set( MIB_WLAN_MACAC_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_ac;
		}

		strVal = get_cstream_var(postData,len, ("mac"), "");
		if ( !strVal[0] ) {
//			strcpy(tmpBuf, ("Error! No mac address to set."));
			goto setac_ret;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_ac;
		}

		strVal = get_cstream_var(postData,len,("comment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_ac;
			}
			strcpy((char *)macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_ac;
		}
		if ( (entryNum + 1) > MAX_WLAN_AC_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_ac;
		}

		//add same rule check
		for(j=1;j<=entryNum;j++)
		{
			memset(&macEntrytmp, 0x00, sizeof(macEntrytmp));
			*((char *)&macEntrytmp) = (char)j;
			if ( apmib_get(MIB_WLAN_MACAC_ADDR, (void *)&macEntrytmp))
			{
				if (!memcmp(macEntrytmp.macAddr, macEntry.macAddr, 6))
				{
					strcpy(tmpBuf, ("rule already exist!"));
					goto setErr_ac;
				}
					
			}
		}
		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_AC_ADDR_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_AC_ADDR_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_ac;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_ac;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = get_cstream_var(postData,len, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_MACAC_ADDR, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_ac;
				}
				if ( !apmib_set(MIB_WLAN_AC_ADDR_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_ac;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_AC_ADDR_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_ac;
		}
	}

setac_ret:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif

#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes",2,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M);
	}
#elif defined(WIFI_QUICK_REINIT)
	if(MUST_REBOOT != 1)
	{
		kick_event(WLAN_BRIDGE_EVENT);
		send_redirect_perm(urlbuf);
	}
	else
		OK_MSG(submitUrl);
#else
	OK_MSG(submitUrl);
#endif

	save_cs_to_file();
	
  	return;

setErr_ac:
	ERR_MSG(urlbuf);
}
/*****************************************************************/

int getScheduleInfo(int argc, char **argv)
{
	int	entryNum=0, i;
	SCHEDULE_T entry;
	int everyday=0, hours24=0;
	int dayWeek=0;
	char tmpBuf[200];
	unsigned char buffer[200];
	int isEnabled=0;
	char *strToken;
	int cmpResult=0;
	int index=0;
	char	*name_arg;

	//printf("get parameter=%s\n", argv[0]);
	name_arg = argv[0];
	if (name_arg == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}
   	
	if ( !strcmp(name_arg, ("wlan_state")) ) {
		bss_info bss;
		getWlBssInfo(WLAN_IF, &bss);
		if (bss.state == STATE_DISABLED) 
			strcpy((char *)buffer, "Disabled");
		else
			strcpy((char *)buffer, "Enabled");	
		web_write_chunked( "%s", buffer);
		return 0;
	}else if(!strcmp(name_arg, ("system_time"))){
		#ifdef HOME_GATEWAY
					return 0;
		#else
		
		return web_write_chunked("%s","menu.addItem(\"System Time\", \"time.htm\", \"\", \"Setup System Time\");");
		#endif
	} 		
	cmpResult= strncmp(name_arg, "getentry_", 9);
	strToken=strstr(name_arg, "_");
	
	index= atoi(strToken+1);
	
	if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)) {
  		strcpy(tmpBuf, "Get table entry error!");
		return -1;
	}
	apmib_get(MIB_WLAN_SCHEDULE_ENABLED,(void *)&isEnabled);
	if(isEnabled==0){
		web_write_chunked("%s", "wlanSchedule-0-0-0-0-0-0");
		return 0;
	}
		
		for (i=1; i<=entryNum; i++) {
				*((char *)&entry) = (char)i;
				if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL, (void *)&entry)){
					fprintf(stderr,"Get SCHEDULE entry fail\n");
					return -1;
				}
				if(entry.eco & ECO_EVERYDAY_MASK)
					everyday = 1;
				else
					everyday = 0;
				
				if(entry.eco & ECO_24HOURS_MASK)
					hours24 = 1;
				else
					hours24 = 0;
					
				if(everyday == 1)
				{
					dayWeek = 127; /* 01111111 */
				}
				else
				{
					dayWeek=entry.day;					
				}
				
				if(hours24 == 1)
				{
					entry.fTime=0;
					entry.tTime=1435;
				}
				
				if(index==i){
					web_write_chunked("%s-%d-%d-%d-%d-%d-%d",entry.text, isEnabled, everyday, dayWeek, hours24, entry.fTime, entry.tTime);   
				}
		}
	return 0;
}

int wlAcList(int argc, char ** argv)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[100];

	if ( !apmib_get(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += web_write_chunked(("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlactrl_fmwlan_macaddr)</script></b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlactrl_comment)</script></b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlactrl_fmwlan_select)</script></b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_WLAN_MACAC_ADDR, (void *)&entry))
			return -1;

		snprintf(tmpBuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent += web_write_chunked( ("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, entry.comment, i);
	}
	return nBytesSent;
}
#if defined(CONFIG_RTL_8881A_SELECTIVE)
void formWlanBand2G5G(char *postData, int len)
{
	char *strVal, *submitUrl;
	int wlanBand2G5GSelect;
	char tmpBuf[100]={0};

	strVal= get_cstream_var(postData,len, ("wlBandMode"), "");
	if(strVal[0]){
		wlanBand2G5GSelect = atoi(strVal);
	}
	if(wlanBand2G5GSelect<BANDMODE2G || wlanBand2G5GSelect>BANDMODESINGLE)
	{
		sprintf(tmpBuf,"Invalid wlanBand2G5GSelect value:%d\n",wlanBand2G5GSelect);
		goto setErr;
	}
	else
	{	
		printf("[%s]   %d  wlanBand2G5GSelect is %d \n",__FUNCTION__,__LINE__,wlanBand2G5GSelect);	
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
	}
	
	submitUrl = get_cstream_var(postData,len, ("submit-url"), ""); 
	
	OK_MSG(submitUrl);
	return;
setErr:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef RTL_MULTI_REPEATER_MODE_SUPPORT
    #define MULTIREPEATER_ENABLE   1
    #define MULTIREPEATER_DISABLE  0
    static int multirepeaterWlan0Flag = 0;
    static int multirepeaterWlan1Flag = 0;
    void set_multirepeaterFlag(int wlanidx,int isEnable)
    {
        if(0==wlanidx)
            multirepeaterWlan0Flag=isEnable;
        else
            multirepeaterWlan1Flag=isEnable;
    }
    int get_multirepeaterFlag(int wlanidx)
    {
        if(0==wlanidx)
            return multirepeaterWlan0Flag;
        else
            return multirepeaterWlan1Flag;
    }
#endif

/******************************************************************/
#ifdef MBSSID
void formWlanMultipleAP(char *postData, int len)
{
	char *strVal, *submitUrl;
	int idx, disabled, old_vwlan_idx, band_no, val;
	char varName[30];
	char redirectUrl[200];
    int repeater_enabled=0;
    int valTmp=0;

//displayPostDate(wp->post_data);	
	submitUrl = get_cstream_var(postData,len, ("submit-url"), ""); 
	sprintf(redirectUrl,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
	old_vwlan_idx = apmib_get_vwlanidx();
#ifdef RTL_MULTI_REPEATER_MODE_SUPPORT         
        sprintf(varName, "switch_multi_repeater");
        strVal= get_cstream_var(postData,len, varName, "");
        if ( !strcmp(strVal, "ON")){
            
            repeater_enabled = 2;
    
    
            /*let root  interface under AP mode ; need make sure vwlan_idx==0*/ 
            valTmp=AP_MODE;
            apmib_set(MIB_WLAN_MODE, (void *)&valTmp);          // set to 0     (AP mode)
            apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&valTmp);  // set to 0   (disabled is false)    
            /*let root  interface under AP mode ; need make sure vwlan_idx==0*/ 

            #ifdef WLAN_PROFILE
            valTmp = 0; // disable AP Profile
    		if(apmib_get_wlanidx() == 0) {
                apmib_set(MIB_PROFILE_ENABLED1, (void *)&valTmp);
            } else {
                apmib_set(MIB_PROFILE_ENABLED2, (void *)&valTmp);
            }
            #endif
    
            /*let vxd interface be disabled*/ 
            apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);//vwlan_idx = NUM_VWLAN_INTERFACE; 
            valTmp=1;
            apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&valTmp);  // set to 1   (disabled is tue)            
             SDEBUG("set vxd (vwlan_idx=[%d]) wlan disabled to true\n",apmib_get_vwlanidx());        
            /*let vxd interface be disabled*/         
    
    
            for (idx=1; idx<=4; idx++) {
                apmib_set_vwlanidx(idx); //vwlan_idx = idx; 
                if(idx==1)
                    valTmp=AP_MODE;
                if(idx==2||idx==3)
                    valTmp=CLIENT_MODE;            
                apmib_set(MIB_WLAN_MODE, (void *)&valTmp);        
    
                #ifdef CONFIG_RTL_MULTI_CLONE_SUPPORT    
                    // let wlanx-va1; wlanx-va2 default enabled its macclone
                    if(idx==2 || idx==3){
                        valTmp=1;
                        apmib_set(MIB_WLAN_MACCLONE_ENABLED, (void *)&valTmp);  // set to 1   (disabled is tue)            
                    }
                #endif
                /*let  4 th vap interface be disabled*/                     
                if(idx==4){
                    valTmp=1;
                    apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&valTmp);  // set to 1   (disabled is tue)            
                }
            }        
            
        } else {
            repeater_enabled = 0;   
            for (idx=1; idx<=3; idx++) {
                apmib_set_vwlanidx(idx);//vwlan_idx = idx; 
                valTmp=AP_MODE;
                apmib_set(MIB_WLAN_MODE, (void *)&valTmp);        
            }
        }
    
        /*let vxd interface be disabled*/ 
        if(apmib_get_wlanidx()==0){
            apmib_set(MIB_REPEATER_ENABLED1, (void *)&repeater_enabled);
            SDEBUG("MIB_REPEATER_ENABLED1 mode set to [%d]\n",repeater_enabled);
        }else{
            apmib_set(MIB_REPEATER_ENABLED2, (void *)&repeater_enabled);
            SDEBUG("MIB_REPEATER_ENABLED2 mode set to [%d]\n",repeater_enabled);
        }            
        
#endif 

	for (idx=1; idx<=4; idx++) {		
		apmib_set_vwlanidx(idx);		

		sprintf(varName, "wl_disable%d", idx);		
		strVal = get_cstream_var(postData,len, varName, "");
		if ( !strcmp(strVal, "ON"))
			disabled = 0;
		else
			disabled = 1;	
		apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&disabled);

		if (disabled)
			continue;
		
		sprintf(varName, "wlanIdx");	
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]){
			if( strVal[0] == '0' )//5G
				val = 2;
			else//2G
				val = 1;
			apmib_set(MIB_WLAN_PHY_BAND_SELECT, (void *)&val);
		}

		sprintf(varName, "wl_band%d", idx);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			int wlan_onoff_tkip;
			band_no = strtol( strVal, (char **)NULL, 10);
			val = (band_no + 1);
			apmib_get( MIB_WLAN_11N_ONOFF_TKIP, (void *)&wlan_onoff_tkip);
				
			if(wlan_onoff_tkip == 0) //Wifi request
			{
				int wpaCipher;
				int wpa2Cipher;
				int wlan_encrypt=0;
				
				apmib_get( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt);
				apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
				apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
				
				if( band_no == 7 || band_no == 9 || band_no == 10 || band_no == 11|| band_no == 63 || band_no == 71 || band_no == 75) //7:n; 9:gn; 10:bgn 11:5g_an
				{
					
					if(wlan_encrypt ==ENCRYPT_WPA || wlan_encrypt ==ENCRYPT_WPA2){
						wpaCipher &= ~WPA_CIPHER_TKIP;
							if(wpaCipher== 0)
								wpaCipher =  WPA_CIPHER_AES;
						apmib_set(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wpaCipher);
						
						wpa2Cipher &= ~WPA_CIPHER_TKIP;
							if(wpa2Cipher== 0)
								wpa2Cipher =  WPA_CIPHER_AES;
						apmib_set(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wpa2Cipher);
					}
					
				}		
			}
		
	
			apmib_set(MIB_WLAN_BAND, (void *)&val);
		}

		sprintf(varName, "wl_ssid%d", idx);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) 
			apmib_set( MIB_WLAN_SSID, (void *)strVal);			
	
		sprintf(varName, "TxRate%d", idx);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0' ) { // auto
				val = 1;
				apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val);
			}
			else  {
				val = 0;
				apmib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&val);
				val = atoi(strVal);
				
				if(val<30)
				val = 1 << (val-1);
				else
					val = ((1<<31) + (val -30));				

				apmib_set(MIB_WLAN_FIX_RATE, (void *)&val);
			}
		}

		sprintf(varName, "wl_hide_ssid%d", idx);
		strVal = get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_HIDDEN_SSID, (void *)&val);
		}
	
		sprintf(varName, "wl_wmm_capable%d", idx);
		strVal= get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val);
		}
		else {	//enable wmm in 11N mode always
			int cur_band;
			apmib_get(MIB_WLAN_BAND, (void *)&cur_band);
			if(cur_band == 10 || cur_band ==11) {
				val = 1;
				apmib_set(MIB_WLAN_WMM_ENABLED, (void *)&val);
			}
		}

		sprintf(varName, "wl_access%d", idx);
		strVal= get_cstream_var(postData,len, varName, "");
		if (strVal[0]) {
			if ( strVal[0] == '0')
				val = 0;
			else 
				val = 1;
			apmib_set(MIB_WLAN_ACCESS, (void *)&val);
		}

		// force basic and support rate to zero to let driver set default
		val = 0;
		apmib_set(MIB_WLAN_BASIC_RATES, (void *)&val);		
		apmib_set(MIB_WLAN_SUPPORTED_RATES, (void *)&val);
		
		apmib_set_vwlanidx (old_vwlan_idx);		
	}

	apmib_set_vwlanidx(old_vwlan_idx);

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif

	
#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes", 10,redirectUrl);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M|SYS_BRIDGE_M|SYS_WIFI_APP_M);
	}
#else	
	//memset(redirectUrl,0x00,sizeof(redirectUrl));
	//sprintf(redirectUrl,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl,apmib_get_wlanidx());
	
	OK_MSG(submitUrl);
#endif
}
#endif

void set_11ac_txrate(WLAN_STA_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRates-0x90;
	if(!txrate)
		return;
	if(pInfo->ht_info & 0x4)
		channelWidth=2;
	else if(pInfo->ht_info & 0x1)
		channelWidth=1;
	else
		channelWidth=0;
	if(pInfo->ht_info & 0x2)
		shortGi=1;

	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channelWidth][shortGi][rate_idx]>>1);
}

/////////////////////////////////////////////////////////////////////////////

int wirelessClientList(int argc, char **argv)
{
	int nBytesSent=0, i, found=0;
	WLAN_STA_INFO_Tp pInfo;
	char *buff;
	char mode_buf[20];
	char txrate[20];
	int rateid=0;

	buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

#ifdef MBSSID
	char Root_WLAN_IF[20];

	if (argc == 2) {
		int virtual_index;
		char virtual_name[20];
		strcpy(Root_WLAN_IF, WLAN_IF);
		virtual_index = atoi(argv[argc-1]) - 1;

#ifdef CONFIG_RTL8196B_GW_8M
		if (virtual_index > 0)
		{
			free(buff);		
			return 0;
		}
#endif
		if(virtual_index >= DEF_MSSID_NUM)
		{
			free(buff);
			return 0;
		}
		sprintf(virtual_name, "-va%d", virtual_index);
		strcat(WLAN_IF, virtual_name);
	}
#endif

	if ( getWlStaInfo(WLAN_IF,  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");
#ifdef MBSSID
		if (argc == 2)
			strcpy(WLAN_IF, Root_WLAN_IF);
#endif
		free(buff);
		return 0;
	}

#ifdef MBSSID
	if (argc == 2)
		strcpy(WLAN_IF, Root_WLAN_IF);
#endif

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			
		if(pInfo->network & BAND_11N)
			sprintf(mode_buf, "%s", (" 11n"));
		else if (pInfo->network & BAND_11G)
			sprintf(mode_buf,"%s",  (" 11g"));	
		else if (pInfo->network & BAND_11B)
			sprintf(mode_buf, "%s", (" 11b"));
		else if (pInfo->network& BAND_11A)
			sprintf(mode_buf, "%s", (" 11a"));
		else if (pInfo->network& BAND_11AC)
			sprintf(mode_buf, "%s", (" 11ac"));
		else
			sprintf(mode_buf, "%s", (" ---"));	
		
		//printf("\n\nthe sta txrate=%d\n\n\n", pInfo->txOperaRates);
		
		if(pInfo->txOperaRates >= 0x90) {
			set_11ac_txrate(pInfo, txrate);
		} else if((pInfo->txOperaRates & 0x80) != 0x80){	
			if(pInfo->txOperaRates%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRates/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRates/2); 
			}
		}else{
			if((pInfo->ht_info & 0x1)==0){ //20M
				if((pInfo->ht_info & 0x2)==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if((pInfo->ht_info & 0x1)==0x1){//40M
				if((pInfo->ht_info & 0x2)==0){//long
					
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
							break;
						}
					}
				}else if((pInfo->ht_info & 0x2)==0x2){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRates){
							sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}
			
		}	
			nBytesSent += web_write_chunked(	
	   		("<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%s</td>"
			"<td><font size=2>%d</td>"		
			"</tr>"),
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			mode_buf,
			pInfo->tx_packets, pInfo->rx_packets,
			txrate,
			( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "yes" : "no"),
			pInfo->expired_time/100
			);
			found++;
		}
	}
	if (found == 0) {
		nBytesSent += web_write_chunked(
	   		("<tr bgcolor=#b7b7b7><td><font size=2>None</td>"
			"<td><font size=2>---</td>"
	     		"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"<td><font size=2>---</td>"
			"</tr>"));
	}

	free(buff);

	return nBytesSent;
}

int wlSiteSurveyTbl(int argc, char **argv)
{
	int nBytesSent=0, i;
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	int mesh_enable; 
#endif 
	BssDscr *pBss;
	char tmpBuf[100], ssidbuf[64], tmp1Buf[10], tmp2Buf[20], wpa_tkip_aes[20],wpa2_tkip_aes[20],ssidHtmlBuf[128]={0};
	//unsigned char* buf = (unsigned char*)malloc(250);
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	char meshidbuf[40] ;
#endif 

	WLAN_MODE_T mode;
	bss_info bss;

#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	if (strstr(WLAN_IF,"wlan") == 0) {
		return 0;
	}
#endif

	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return 0;
		}
	}

	pStatus->number = 0; // request BSS DB

	if ( getWlSiteSurveyResult(WLAN_IF, pStatus) < 0 ) {
		//ERR_MSG("Read site-survey status failed!");
		web_write_chunked("Read site-survey status failed!");
		free(pStatus); //sc_yang
		pStatus = NULL;
		return 0;
	}

	if ( !apmib_get( MIB_WLAN_MODE, (void *)&mode) ) {
		printf("Get MIB_WLAN_MODE MIB failed!");
		return 0;
	}
#ifdef CONFIG_RTK_MESH
// ==== inserted by GANTOE for site survey 2008/12/26 ====
	mesh_enable = mode > 3 ? 1 : 0;	// Might to be corrected after the code refinement
#endif
	if ( getWlBssInfo(WLAN_IF, &bss) < 0) {
		printf("Get bssinfo failed!");
		return 0;
	}

// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
//#ifdef CONFIG_RTK_MESH
#if 0
	if(mesh_enable) 
	{ 
		nBytesSent += req_format_write(wp, ("<tr>" 
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MESHID</b></font></td>\n" 
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC ADDR</b></font></td>\n" 
		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n")); 
		nBytesSent += req_format_write(wp, ("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n")); 

		for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) 
		{ 
			pBss = &pStatus->bssdb[i]; 
			if(pBss->bdMeshId.Length == 0)
				continue; 

			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length); // the problem of padding still exists 
			meshidbuf[pBss->bdMeshId.Length] = '\0'; 

			snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"), 
				pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2], 
				pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]); 
			memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length); 
			ssidbuf[pBss->bdSsId.Length] = '\0'; 
			
			nBytesSent += req_format_write(wp, ("<tr>" 
				"<td align=left width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"), 
				meshidbuf, tmpBuf, pBss->ChannelNumber); 
            
			nBytesSent += req_format_write(wp, 
			("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=" 
				"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n"), i); 
		} 
	} 
	else 
#endif 
	{ 
		int rptEnabled=0;
#if defined(CONFIG_SMART_REPEATER)
		int opmode=0;
		apmib_get(MIB_OP_MODE, (void *)&opmode);
#endif
		if(apmib_get_wlanidx() == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);

		/*strcpy(buf,"<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ssid)</script></b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_bssid)</script></b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_chan)</script></b></font></td>\n"
    "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_type)</script></b></font></td>\n"
    "<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ency)</script></b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_signal)</script></b></font></td>\n");
		
	nBytesSent += web_write_chunked(buf);*/
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
				nBytesSent += web_write_chunked(("<tr bgcolor=\"#007FCC\">"
	"<td  align=center width=\"20%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_ssid)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"25%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_bssid)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"10%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_chan)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked((  "<td align=center width=\"10%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_type)</script></b></font></td>\n"
    "<td align=center width=\"15%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_ency)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"10%%\" ><font size=\"2\" color=\"#FFFFFF\"><b><script>dw(wlsurvey_tbl_signal)</script></b></font></td>\n"));

#else

nBytesSent += web_write_chunked(("<tr>"
	"<td  align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ssid)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_bssid)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_chan)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked((  "<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_type)</script></b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ency)</script></b></font></td>\n"));
		nBytesSent += web_write_chunked(("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_signal)</script></b></font></td>\n"));
#endif
	/*nBytesSent += web_write_chunked( ("<tr>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ssid)</script></b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_bssid)</script></b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_chan)</script></b></font></td>\n"
    "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_type)</script></b></font></td>\n"
    "<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_ency)</script></b></font></td>\n"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlsurvey_tbl_signal)</script></b></font></td>\n"));*/
	if( (mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE || mode == AP_WDS_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
	)
#endif
	)
	{
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	nBytesSent += web_write_chunked(("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\"><font size=\"2\" color=\"#FFFFFF\"><b>Select</b></font></td></tr>\n"));
#else
		nBytesSent += web_write_chunked(("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
#endif
	}
	else
		nBytesSent += web_write_chunked(("</tr>\n"));

	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length] = '\0';
		htmlSpecialCharReplace(ssidbuf,ssidHtmlBuf,sizeof(ssidHtmlBuf));

#if defined(CONFIG_RTK_MESH)
		if( pBss->bdMeshId.Length )
		{
			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length);	// the problem of padding still exists

			if( !memcmp(ssidbuf, meshidbuf,pBss->bdMeshId.Length-1) )
				continue;
		}
#endif


		if (pBss->network==BAND_11B)
			strcpy(tmp1Buf, (" (B)"));
		else if (pBss->network==BAND_11G)
			strcpy(tmp1Buf, (" (G)"));	
		else if (pBss->network==(BAND_11G|BAND_11B))
			strcpy(tmp1Buf, (" (B+G)"));
		else if (pBss->network==(BAND_11N))
			strcpy(tmp1Buf, (" (N)"));		
		else if (pBss->network==(BAND_11G|BAND_11N))
			strcpy(tmp1Buf, (" (G+N)"));	
		else if (pBss->network==(BAND_11G|BAND_11B | BAND_11N))
			strcpy(tmp1Buf, (" (B+G+N)"));	
		else if(pBss->network== BAND_11A)
			strcpy(tmp1Buf, (" (A)"));
		else if(pBss->network== (BAND_11A | BAND_11N))
			strcpy(tmp1Buf, (" (A+N)"));
#if 1  // Open it for all platform
//#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		else if(pBss->network== (BAND_11A | BAND_11N | BAND_11AC))
			strcpy(tmp1Buf, (" (A+N+AC)"));
		else if(pBss->network== (BAND_11N | BAND_11AC))
			strcpy(tmp1Buf, (" (N+AC)"));	
		else if(pBss->network== (BAND_11A | BAND_11AC))
			strcpy(tmp1Buf, (" (A+AC)"));	
		else if(pBss->network == (BAND_11B | BAND_11G | BAND_11N | BAND_11AC))
			strcpy(tmp1Buf,("(B+G+N+AC)"));
#endif
		else
			sprintf(tmp1Buf, (" -%d-"),pBss->network);

		memset(wpa_tkip_aes,0x00,sizeof(wpa_tkip_aes));
		memset(wpa2_tkip_aes,0x00,sizeof(wpa2_tkip_aes));
		
		if ((pBss->bdCap & cPrivacy) == 0)
			sprintf(tmp2Buf, "no");
		else {
			if (pBss->bdTstamp[0] == 0)
				sprintf(tmp2Buf, "WEP");
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			else if (pBss->bdTstamp[0] == SECURITY_INFO_WAPI)
				sprintf(tmp2Buf, "WAPI");
#endif
			else {
				int wpa_exist = 0, idx = 0;
				if (pBss->bdTstamp[0] & 0x0000ffff) {
					idx = sprintf(tmp2Buf, "WPA");
					if (((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if(((pBss->bdTstamp[0] & 0x0000f000) >> 12) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");
					wpa_exist = 1;

					if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x5)
						sprintf(wpa_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x4)
						sprintf(wpa_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x00000f00) >> 8) == 0x1)
						sprintf(wpa_tkip_aes,"%s","tkip");
				}
				if (pBss->bdTstamp[0] & 0xffff0000) {
					if (wpa_exist)
						idx += sprintf(tmp2Buf+idx, "/");
					idx += sprintf(tmp2Buf+idx, "WPA2");
					if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x4)
						idx += sprintf(tmp2Buf+idx, "-PSK");
					else if (((pBss->bdTstamp[0] & 0xf0000000) >> 28) == 0x2)
						idx += sprintf(tmp2Buf+idx, "-1X");

					if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x5)
						sprintf(wpa2_tkip_aes,"%s","aes/tkip");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x4)
						sprintf(wpa2_tkip_aes,"%s","aes");
					else if (((pBss->bdTstamp[0] & 0x0f000000) >> 24) == 0x1)
						sprintf(wpa2_tkip_aes,"%s","tkip");
				}
			}
		}

#if 0
		if( mesh_enable && (pBss->bdMeshId.Length > 0) )
		{
			nBytesSent += req_format_write(wp, ("<tr>"
			"<td align=left width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d%s</td>\n"     
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"),
			ssidbuf, tmpBuf, pBss->ChannelNumber, tmp1Buf, "Mesh Node", tmp2Buf, pBss->rssi);
		}
		else
#endif
		{
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
			if(i%2)
				nBytesSent += web_write_chunked("<tr bgcolor=\"#FFFFFF\">\n");
			else
				nBytesSent += web_write_chunked("<tr bgcolor=\"#DDDDDD\">\n");
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;'  align=center width=\"20%%\"\
			><font size=\"2\">%s</font></td>\n",ssidHtmlBuf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"25%%\" \
			><font size=\"2\">%s</font></td>\n",tmpBuf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			><font size=\"2\">%d<br/>%s</font></td>\n",pBss->ChannelNumber, tmp1Buf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			><font size=\"2\">%s</font></td>\n",((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"));
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"15%%\" \
			><font size=\"2\">%s</font></td>\n",tmp2Buf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			><font size=\"2\">%d</font></td>\n",pBss->rssi);
#else
			
			nBytesSent += web_write_chunked("<tr>\n");
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;'  align=center width=\"20%%\"\
			bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",ssidHtmlBuf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"25%%\" \
			bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",tmpBuf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			bgcolor=\"#C0C0C0\"><font size=\"2\">%d<br/>%s</font></td>\n",pBss->ChannelNumber, tmp1Buf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"));
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"15%%\" \
			bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n",tmp2Buf);
			nBytesSent += web_write_chunked("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" \
			bgcolor=\"#C0C0C0\"><font size=\"2\">%d</font></td>\n",pBss->rssi);
#endif
		}
		/*diag_printf(("<tr>"
			"<td style='word-break : break-all; overflow:hidden;'  align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d<br/>%s</font></td>\n"     
      		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</font></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</font></td>\n %s:%d"),ssidbuf, tmpBuf, pBss->ChannelNumber, tmp1Buf,
			((pBss->bdCap & cIBSS) ? "Ad hoc" : "AP"), tmp2Buf, pBss->rssi,__FUNCTION__,__LINE__);*/

		if( ( mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE || mode == AP_WDS_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
		)
#endif

		)
		{
			//diag_printf("ssidbuf=%s %s:%d\n",ssidHtmlBuf,__FUNCTION__,__LINE__);
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
			nBytesSent += web_write_chunked("<td align=center width=\"10%%\" >\n");

#else
			nBytesSent += web_write_chunked("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\">\n");
#endif
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"selSSID_VAL_%d\" value=\"%s\">\n",i,ssidHtmlBuf);
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"selSSID_%d\" value=\"%s\">\n",i,tmpBuf);
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"selChannel_%d\" value=\"%d\">\n",i,pBss->ChannelNumber);
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"selEncrypt_%d\" value=\"%s\" >\n",i,tmp2Buf);
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"wpa_tkip_aes_%d\" value=\"%s\" >\n",i,wpa_tkip_aes);
			nBytesSent += web_write_chunked("<input type=\"hidden\" id=\"wpa2_tkip_aes_%d\" value=\"%s\" >\n",i,wpa2_tkip_aes);
			nBytesSent += web_write_chunked("<input type=\"radio\" name=\"select\" value=\"sel%d\" onClick=\"enableConnect(%d)\">\n",i,i);
			nBytesSent += web_write_chunked("</td></tr>\n");
		}
		else
			nBytesSent += web_write_chunked(("</tr>\n"));
	}

	if( pStatus->number == 0 )
	{
		if (( mode == CLIENT_MODE )
#if defined(CONFIG_RTL_ULINKER)
		|| (mode == AP_MODE && rptEnabled == 1)
#endif
#if defined(CONFIG_SMART_REPEATER)
		||( (mode == AP_MODE || mode == AP_WDS_MODE) && (rptEnabled == 1) 
		//&& (opmode == WISP_MODE)
		)
#endif
		)
		{
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
		nBytesSent +=web_write_chunked( ("<tr bgcolor=\"#DDDDDD\">"
#else
			nBytesSent +=web_write_chunked( ("<tr>"
#endif
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"));
	        nBytesSent +=web_write_chunked( ("<td style='word-break : break-all; overflow:hidden;' align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "<td style='word-break : break-all; overflow:hidden;' align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
	                "</tr>\n"));
		}
		else
		{
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
			nBytesSent += web_write_chunked(("<tr bgcolor=\"#DDDDDD\">"
#else
			nBytesSent += web_write_chunked(("<tr>"
#endif
			"<td style='word-break : break-all; overflow:hidden;' align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
			"<td style='word-break : break-all; overflow:hidden;' align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"));
			nBytesSent += web_write_chunked(("<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td style='word-break : break-all; overflow:hidden;' align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td style='word-break : break-all; overflow:hidden;' align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"</tr>\n"));
		}
	}
	nBytesSent += web_write_chunked(("</table>\n"));

#ifdef CONFIG_RTK_MESH
	if(mesh_enable) 
	{ 
		int mesh_count = 0;

		nBytesSent +=web_write_chunked(("<table border=\"1\" width=\"500\">"
		"<tr><h4><font><br><br>List of Mesh Points</font></tr><tr>"
		"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Mesh ID</b></font></td>\n" 
		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Adddress</b></font></td>\n" 
		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Channel</b></font></td>\n")); 
		nBytesSent += web_write_chunked("<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n");

		for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) 
		{
			pBss = &pStatus->bssdb[i]; 
			if(pBss->bdMeshId.Length == 0)
				continue; 
			mesh_count++;
			memcpy(meshidbuf, pBss->bdMeshIdBuf - 2, pBss->bdMeshId.Length); // the problem of padding still exists
			meshidbuf[pBss->bdMeshId.Length] = '\0'; 

			snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"), 
				pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2], 
				pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]); 
			memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length); 
			ssidbuf[pBss->bdSsId.Length] = '\0'; 
			
			nBytesSent += web_write_chunked(("<tr>" 
				"<td align=left width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n" 
				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"), 
				meshidbuf, tmpBuf, pBss->ChannelNumber); 
            
			nBytesSent += web_write_chunked(
			("<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=" 
				"\"select\" value=\"sel%d\" onClick=\"enableConnect()\"></td></tr>\n"), i); 
		}
		if( mesh_count == 0 )
		{
			nBytesSent += web_write_chunked(("<tr>"
			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><pre><font size=\"2\">None</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"></td>\n"));
		}
		nBytesSent += web_write_chunked(("</table>")); 
	}
#endif
	} 
	return nBytesSent;
}

int wlWdsList(int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WDS_T entry;
	char tmpBuf[100];
	char txrate[20];
	int rateid=0;

	if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	nBytesSent += web_write_chunked(("<tr>"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlwds_mac_addr)</script></b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlwds_fmwlan_txrate)</script></b></font></td>\n"
      	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlwds_fmwlan_comment)</script></b></font></td>\n"
      	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b><script>dw(wlwds_fmwlan_select)</script></b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_WLAN_WDS, (void *)&entry))
			return -1;

		snprintf(tmpBuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		if(entry.fixedTxRate == 0){	
				sprintf(txrate, "%s","Auto"); 
		}else{
			for(rateid=0; rateid<48;rateid++){
				if(tx_fixed_rate[rateid].id == entry.fixedTxRate){
					sprintf(txrate, "%s", tx_fixed_rate[rateid].rate);
					break;
				}
			}
		}	
		nBytesSent += web_write_chunked(("<tr>"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, txrate, entry.comment,i);
	}
	return nBytesSent;
}

int sitesurvey_connect_wepHandler(char *postData, int len, char *tmpBuf, int wlan_id)
{
	char  *wepKey;
	char *strKeyLen, *strFormat, *strAuth, /* *strKeyId, */ *strEnabled;
	char key[30];
	int val, keyLen, ret, i;
	WEP_T wep;
	ENCRYPT_T encrypt=ENCRYPT_WEP;
	char varName[32];
	int wlanMode, rptEnabled;
	int authType;

	sprintf(varName, "length%d", wlan_id);
	strKeyLen =get_cstream_var(postData,len, varName, "");
	if (!strKeyLen[0]) {
		strcpy(tmpBuf, ("Key length must exist!"));
		goto setErr_wep;
	}
	if (strKeyLen[0]!='1' && strKeyLen[0]!='2') {
		strcpy(tmpBuf, ("Invalid key length value!"));
		goto setErr_wep;
	}
	if (strKeyLen[0] == '1') {
		wep = WEP64;
		val= 1;
	}
	else {
		wep = WEP128;
		val = 5;
	}

	sprintf(tmpBuf, "%s=%d", "encmode", val);
	RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

	sprintf(varName, "authType%d", wlan_id);
	strAuth = get_cstream_var(postData,len, varName, "");
	if (strAuth[0]) { // new UI
		if (!strcmp(strAuth, ("open")))
			authType = AUTH_OPEN;
		else if ( !strcmp(strAuth, ("shared")))
			authType = AUTH_SHARED;
		else 
			authType = AUTH_BOTH;

		sprintf(tmpBuf, "%s=%d", "authtype", authType);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
	}
	else {
		authType = AUTH_BOTH;
		sprintf(tmpBuf, "%s=%d", "authtype", authType);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
	}
				
	sprintf(varName, "format%d", wlan_id);
	strFormat = get_cstream_var(postData,len, varName, "");
	if (!strFormat[0]) {
		strcpy(tmpBuf, ("Key type must exist!"));
		goto setErr_wep;
	}

	if (strFormat[0]!='1' && strFormat[0]!='2') {
		strcpy(tmpBuf, ("Invalid key type value!"));
		goto setErr_wep;
	}

	if (wep == WEP64) {
		if (strFormat[0]=='1')
			keyLen = WEP64_KEY_LEN;
		else
			keyLen = WEP64_KEY_LEN*2;
	}
	else {
		if (strFormat[0]=='1')
			keyLen = WEP128_KEY_LEN;
		else
			keyLen = WEP128_KEY_LEN*2;
	}

	sprintf(varName, "key%d", wlan_id);
	wepKey = get_cstream_var(postData,len, varName, "");

	if	(wepKey[0]) {
		if (strlen(wepKey) != keyLen) {
			sprintf(tmpBuf, ("Invalid key length! Expect length is %d"), keyLen);
			goto setErr_wep;
		}
		if ( !isAllStar(wepKey) ) {
			if (strFormat[0] == '1'){ // ascii
				if (wep == WEP64)
					sprintf(key, "%02x%02x%02x%02x%02x", wepKey[0], wepKey[1], wepKey[2], wepKey[3], wepKey[4]);
				else
					sprintf(key, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
						wepKey[0], wepKey[1], wepKey[2], wepKey[3], wepKey[4],
						wepKey[5], wepKey[6], wepKey[7], wepKey[8], wepKey[9], wepKey[10], wepKey[11], wepKey[12]);	
			}
			else { // hex
				strcpy(key, wepKey);			
			}

			sprintf(tmpBuf, "%s=%d", "wepdkeyid", 0);
			RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);

			sprintf(tmpBuf, "%s=%s", "wepkey1", key);
			RunSystemCmd(NULL_FILE, "iwpriv", "wlan0-vxd0", "set_mib", tmpBuf, NULL_STR);
		}
	}
	return 0 ;
setErr_wep:

	return -1 ; 
}

//return 0: connect ok, 1:connect fail
int sitesurvey_connect_test(char *tmpBuf, int val, int encrypt)
{
	unsigned char res = val;
	int wait_time = 0, max_wait_time = 5 ,status = 0, ret = 1;

	printf("[%s:%d] ssid[%s], channel is %d\n", __FUNCTION__, __LINE__, pStatus->bssdb[val].bdSsIdBuf, pStatus->bssdb[val].ChannelNumber);//bruce

	/* set switch_chan */
	sprintf(tmpBuf, "%s=1,%d", "switch_chan", pStatus->bssdb[val].ChannelNumber);
	RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpBuf, NULL_STR);
	
	while (1) {
		if ( getWlJoinRequest("wlan0-vxd0", &pStatus->bssdb[val], &res) < 0 ) {
			strcpy(tmpBuf, ("Join request failed!"));
			goto CONNECT_ERR0;
		}
		if ( res == 1 ) { // wait
			if (wait_time++ > 5) {
				strcpy(tmpBuf, ("connect-request timeout!"));
				goto CONNECT_ERR0;
			}
			sleep(1);
			continue;
		}
		break;
	}
#if 0
	if ( res == 2 ) // invalid index
		strcpy(tmpBuf, ("Connect failed 3!"));
	else 
#endif
	{
		wait_time = 0;
		while (1) {
			if (getWlJoinResult("wlan0-vxd0", &res) < 0) {
				strcpy(tmpBuf, ("Get Join result failed!"));
				goto CONNECT_ERR0;
			}
			if(res==STATE_Bss || res==STATE_Ibss_Idle || res==STATE_Ibss_Active) // completed 
				break;
			else {
				if (wait_time++ > 10) {
					strcpy(tmpBuf, ("connect timeout!"));
					goto CONNECT_ERR0;
				}
			}
			sleep(1);
		}

		if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2) {
			bss_info bss;
			wait_time = 0;
			max_wait_time=10; //Need more test, especially for 802.1x client mode
			
			while (wait_time++ < max_wait_time) {
				getWlBssInfo("wlan0-vxd0", &bss);
				if (bss.state == STATE_CONNECTED)
					break;
				sleep(1);
			}
			if (wait_time > max_wait_time)						
				status = 1;
		}
		if (status) {
			strcpy(tmpBuf, ("Connect failed 5!"));
			goto CONNECT_ERR0;
		}
		else
			strcpy(tmpBuf, ("Connect successfully!"));
	}

	ret = 0;
CONNECT_ERR0:
	/* restore switch_chan */
	RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "switch_chan=0", NULL_STR);
	return ret;
}


void formWlWds(char *postData, int len)
{
	char *strRate, *strAddMac, *strDelMac, *strDelAllMac, *strVal, *submitUrl, *strEnabled;
	char tmpBuf[100];
	char urlbuf[60];
	int entryNum, i, enabled, val, j;
	WDS_T macEntry, macEntrytmp;

	int maxWDSNum;

	submitUrl = get_cstream_var(postData,len, ("submit-url"), "");   // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
#ifdef CONFIG_RTL8196B_GW_8M
	maxWDSNum = 4;
#else
	maxWDSNum = MAX_WDS_NUM;
#endif

	strAddMac = get_cstream_var(postData,len, ("addWdsMac"), "");
	strDelMac = get_cstream_var(postData,len, ("deleteSelWdsMac"), "");
	strDelAllMac = get_cstream_var(postData,len, ("deleteAllWdsMac"), "");
	strEnabled = get_cstream_var(postData,len, ("wlanWdsEnabled"), "");

	if (strAddMac[0]) {
		if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0;
		if ( apmib_set( MIB_WLAN_WDS_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_wds;
		}

		strVal = get_cstream_var(postData,len, ("mac"), "");
		if ( !strVal[0] )
			goto setWds_ret;

		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_wds;
		}

		strVal = get_cstream_var(postData,len, ("comment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_wds;
			}
			strcpy( (char *)macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';


		
		strRate = get_cstream_var(postData,len, "txRate", "");
		if ( strRate[0] ) {
			if ( strRate[0] == '0' ) { // auto
				macEntry.fixedTxRate =0;
			}else  {
				val = atoi(strRate);

				printf("\n\nwds txRate val=%d\n\n", val);

				if(val < 30)
				val = 1 << (val-1);
				else
					val = ((1 <<31) + (val-30) ); 
					
				macEntry.fixedTxRate = val;
			}
		}
	

		
		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		if ( (entryNum + 1) > maxWDSNum) {
			strcpy(tmpBuf, ("Cannot add new entry because table is full!"));
			goto setErr_wds;
		}
		
		//add same rule check
		for(j=1;j<=entryNum;j++)
		{
			memset(&macEntrytmp, 0x00, sizeof(macEntrytmp));
			*((char *)&macEntrytmp) = (char)j;
			if ( apmib_get(MIB_WLAN_WDS, (void *)&macEntrytmp))
			{
				if (!memcmp(macEntrytmp.macAddr, macEntry.macAddr, 6))
				{
					strcpy(tmpBuf, ("rule already exist!"));
					goto setErr_wds;
				}
					
			}
		}

		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_WDS_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_wds;
		}
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_wds;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = get_cstream_var(postData,len,tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_WDS, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_wds;
				}
				if ( !apmib_set(MIB_WLAN_WDS_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_wds;
				}
			}
		}
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_WDS_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_wds;
		}
	}

setWds_ret:
	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif

	
#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes",2,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M|SYS_BRIDGE_M);
	}
#elif defined(WIFI_QUICK_REINIT)
	if(MUST_REBOOT != 1)
	{
		kick_event(WLAN_BRIDGE_EVENT);
		send_redirect_perm(urlbuf);
	}
	else
		OK_MSG(submitUrl);
#else
	OK_MSG(submitUrl);
#endif


  	return;

setErr_wds:
	ERR_MSG(urlbuf);
}

void formWdsEncrypt(char *postData, int length)
{
   	char *strVal, *submitUrl;
	char tmpBuf[100];
	char urlbuf[60];
	WDS_ENCRYPT_T encrypt;
	int intVal, keyLen=0, oldFormat, oldPskLen, len, i;
	char charArray[16]={'0' ,'1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	char key[100];
	char varName[20];

	submitUrl = get_cstream_var(postData,length, ("submit-url"), "");   // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
	sprintf(varName, "encrypt%d", apmib_get_wlanidx());
	strVal = get_cstream_var(postData,length, varName, "");
	if (strVal[0]) {
		encrypt = strVal[0] - '0';
		if (encrypt != WDS_ENCRYPT_DISABLED && encrypt != WDS_ENCRYPT_WEP64 &&
			encrypt != WDS_ENCRYPT_WEP128 && encrypt != WDS_ENCRYPT_TKIP &&
				encrypt != WDS_ENCRYPT_AES) {
 			strcpy(tmpBuf, ("encrypt value not validt!"));
			goto setErr_wdsEncrypt;
		}
		apmib_set( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
	}
	else
		apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);

	if (encrypt == WDS_ENCRYPT_WEP64 || encrypt == WDS_ENCRYPT_WEP128) {
		sprintf(varName, "format%d", apmib_get_wlanidx());
		strVal = get_cstream_var(postData,length, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, ("Invalid wep key format value!"));
				goto setErr_wdsEncrypt;
		}
			intVal = strVal[0] - '0';
			apmib_set( MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal);
		}
		else
			apmib_get( MIB_WLAN_WDS_WEP_FORMAT, (void *)&intVal);

		if (encrypt == WDS_ENCRYPT_WEP64)
			keyLen = WEP64_KEY_LEN;
		else if (encrypt == WDS_ENCRYPT_WEP128)
			keyLen = WEP128_KEY_LEN;

		if (intVal == 1) // hex
			keyLen <<= 1;

		sprintf(varName, "wepKey%d", apmib_get_wlanidx());
		strVal = get_cstream_var(postData,length, varName, "");
		if  (strVal[0]) {
			if (strlen(strVal) != keyLen) {
				strcpy(tmpBuf, ("Invalid wep key length!"));
				goto setErr_wdsEncrypt;
		}
			if ( !isAllStar(strVal) ) {
				if (intVal == 0) { // ascii
					for (i=0; i<keyLen; i++) {
						key[i*2] = charArray[(strVal[i]>>4)&0xf];
						key[i*2+1] = charArray[strVal[i]&0xf];
				}
					key[i*2] = '\0';
			}
				else  // hex
					strcpy(key, strVal);
				apmib_set( MIB_WLAN_WDS_WEP_KEY, (void *)key);
			}
		}
	}
	if (encrypt == WDS_ENCRYPT_TKIP || encrypt == WDS_ENCRYPT_AES) {
		sprintf(varName, "pskFormat%d", apmib_get_wlanidx());
		strVal = get_cstream_var(postData,length, varName, "");
		if (strVal[0]) {
			if (strVal[0]!='0' && strVal[0]!='1') {
				strcpy(tmpBuf, ("Invalid wep key format value!"));
				goto setErr_wdsEncrypt;
				}
			intVal = strVal[0] - '0';
			}
			else
			apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);


		// remember current psk format and length to compare to default case "****"
		apmib_get(MIB_WLAN_WDS_PSK_FORMAT, (void *)&oldFormat);
		apmib_get(MIB_WLAN_WDS_PSK, (void *)tmpBuf);
		oldPskLen = strlen(tmpBuf);

		sprintf(varName, "pskValue%d", apmib_get_wlanidx());
		strVal = get_cstream_var(postData,length, varName, "");
		len = strlen(strVal);
		if (len > 0 && oldFormat == intVal && len == oldPskLen ) {
			for (i=0; i<len; i++) {
				if ( strVal[i] != '*' )
				break;
			}
			if (i == len)
				goto save_wdsEcrypt;
		}
		if (intVal==1) { // hex
			if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
				strcpy(tmpBuf, ("Error! invalid psk value."));
				goto setErr_wdsEncrypt;
	}
				}
		else { // passphras
			if (len==0 || len > (MAX_PSK_LEN-1) || len < MIN_PSK_LEN) {
				strcpy(tmpBuf, ("Error! invalid psk value."));
				goto setErr_wdsEncrypt;
			}
		}
		apmib_set( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		apmib_set( MIB_WLAN_WDS_PSK, (void *)strVal);
	}

save_wdsEcrypt:
	intVal = 1;
	apmib_set( MIB_WLAN_WDS_ENABLED, (void *)&intVal);

	apmib_update_web(CURRENT_SETTING);

#ifndef NO_ACTION
//	run_init_script("bridge");
#endif

#ifdef HAVE_SYSTEM_REINIT
	{
		wait_redirect("Apply Changes",2,urlbuf);
		sleep(1);
		kick_reinit_m(SYS_WIFI_M|SYS_BRIDGE_M);
	}
#elif defined(WIFI_QUICK_REINIT)
	if(MUST_REBOOT != 1)
	{
		kick_event(WLAN_BRIDGE_EVENT);
		send_redirect_perm(submitUrl);
	}
	else
		OK_MSG(submitUrl);
#else
	OK_MSG(submitUrl);
#endif


	return;

setErr_wdsEncrypt:
	ERR_MSG(urlbuf);
}


void set_11ac_wds_txrate(WDS_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRate-0x90;
	int short_gi=0;
	int channel_bandwidth=0;
	apmib_get(MIB_WLAN_SHORT_GI, (void *)&short_gi);
	apmib_get(MIB_WLAN_CHANNEL_BONDING, (void *)&channel_bandwidth);
	if(!txrate)
		return;
	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channel_bandwidth][short_gi][rate_idx]>>1);
}

/////////////////////////////////////////////////////////////////////////////
int wdsList(int argc, char **argv)
{
	int nBytesSent=0, i;
	WDS_INFO_Tp pInfo;
	char *buff;
	char txrate[20];
	int rateid=0;
	int short_gi=0;
	int channel_bandwidth=0;
	

	buff = calloc(1, sizeof(WDS_INFO_T)*MAX_STA_NUM);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return 0;
	}

	if ( getWdsInfo(WLAN_IF, buff) < 0 ) {
		printf("Read wlan sta info failed!\n");
		return 0;
	}

	for (i=0; i<MAX_WDS_NUM; i++) {
		pInfo = (WDS_INFO_Tp)&buff[i*sizeof(WDS_INFO_T)];
		
		if (pInfo->state == STATE_WDS_EMPTY)
			break;
		if(pInfo->txOperaRate >= 0x90){
			set_11ac_wds_txrate(pInfo, txrate);
		}
		else if((pInfo->txOperaRate & 0x80) != 0x80){
			if(pInfo->txOperaRate%2){
				sprintf(txrate, "%d%s",pInfo->txOperaRate/2, ".5"); 
			}else{
				sprintf(txrate, "%d",pInfo->txOperaRate/2); 
			}
		}else{
			apmib_get(MIB_WLAN_CHANNEL_BONDING, (void *)&channel_bandwidth);
			apmib_get(MIB_WLAN_SHORT_GI, (void *)&short_gi);		
			if(channel_bandwidth ==0){ //20M
				if(short_gi==0){//long
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_LONG[rateid].id == pInfo->txOperaRate){
							sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
							break;
						}
					}
				}else if(short_gi==1){//short
					for(rateid=0; rateid<16;rateid++){
						if(rate_11n_table_20M_SHORT[rateid].id == pInfo->txOperaRate){
							sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
							break;
						}
					}
				}
			}else if(channel_bandwidth ==1){ //40
					if(short_gi==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_LONG[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
								break;
							}
						}
					}else if(short_gi==1){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_40M_SHORT[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
								break;
							}
						}
					}	
			}else if(channel_bandwidth ==2){ //80M
					if(short_gi==0){//long
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_80M_LONG[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_80M_LONG[rateid].rate);
								break;
							}
						}
					}else if(short_gi==1){//short
						for(rateid=0; rateid<16;rateid++){
							if(rate_11n_table_80M_SHORT[rateid].id == pInfo->txOperaRate){
								sprintf(txrate, "%s", rate_11n_table_80M_SHORT[rateid].rate);
								break;
							}
						}
					}
			}
		}
		nBytesSent += web_write_chunked(
	   		"<tr bgcolor=#b7b7b7><td><font size=2>%02x:%02x:%02x:%02x:%02x:%02x</td>"
			"<td><font size=2>%d</td>"
	     		"<td><font size=2>%d</td>"
			"<td><font size=2>%d</td>"
			"<td><font size=2>%s</td>",
			pInfo->addr[0],pInfo->addr[1],pInfo->addr[2],pInfo->addr[3],pInfo->addr[4],pInfo->addr[5],
			pInfo->tx_packets, pInfo->tx_errors, pInfo->rx_packets,
			txrate);
	}
	
	free(buff);

	return nBytesSent;
}
/*************************************************************************/
#ifdef CONFIG_RTK_MESH
#define MAX_MSG_BUFFER_SIZE 256
int meshWpaHandler(char *tmpBuf, int length,int wlan_id)
{
  	char *strEncrypt, *strVal;
	ENCRYPT_T encrypt;
	int	intVal, getPSK=1, len;	

	char varName[20];

	sprintf(varName, "method%d", wlan_id);
   	strEncrypt = get_cstream_var(tmpBuf, length, varName, "");
		
	if (!strEncrypt[0]) {
 		strcpy(tmpBuf, ("Error! no encryption method."));
		goto setErr_mEncrypt;
	}
	encrypt = (ENCRYPT_T) strEncrypt[0] - '0';
	if (encrypt!=ENCRYPT_DISABLED &&  encrypt != ENCRYPT_WPA2 ) {
		strcpy(tmpBuf, ("Invalid encryption method!"));
		goto setErr_mEncrypt;
	}

	if (apmib_set( MIB_WLAN_MESH_ENCRYPT, (void *)&encrypt) == 0) {
  		strcpy(tmpBuf, ("Set MIB_WLAN_MESH_ENCRYPT mib error!"));
		goto setErr_mEncrypt;
	}

	if(encrypt == ENCRYPT_WPA2)
	{
		// WPA authentication  ( RADIU / Pre-Shared Key )
		intVal = WPA_AUTH_PSK;		
		if ( apmib_set(MIB_WLAN_MESH_WPA_AUTH, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_MESH_AUTH_TYPE failed!"));
				goto setErr_mEncrypt;
		}

		// cipher suite	 (TKIP / AES)
		intVal =   WPA_CIPHER_AES ;		
		if ( apmib_set(MIB_WLAN_MESH_WPA2_CIPHER_SUITE, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_MESH_WPA2_UNICIPHER failed!"));
				goto setErr_mEncrypt;
		}

		// pre-shared key
		if ( getPSK ) {
			int oldFormat, oldPskLen, i;

			sprintf(varName, "pskFormat%d", wlan_id);
   			strVal = get_cstream_var(tmpBuf, length, varName, "");
			if (!strVal[0]) {
	 			strcpy(tmpBuf, ("Error! no psk format."));
				goto setErr_mEncrypt;
			}
			intVal = strVal[0] - '0';
			if (intVal != 0 && intVal != 1) {
	 			strcpy(tmpBuf, ("Error! invalid psk format."));
				goto setErr_mEncrypt;
			}

			sprintf(varName, "pskValue%d", wlan_id);
			strVal = get_cstream_var(tmpBuf, length, varName, "");		
			len = strlen(strVal);
			
			// remember current psk format and length to compare to default case "****"
			apmib_get(MIB_WLAN_MESH_PSK_FORMAT, (void *)&oldFormat);
			apmib_get(MIB_WLAN_MESH_WPA_PSK, (void *)tmpBuf);
			oldPskLen = strlen(tmpBuf);
			
			if (oldFormat == intVal && len == oldPskLen ) {
				for (i=0; i<len; i++) {
					if ( strVal[i] != '*' )
						break;
				}
				if (i == len)
					goto mRekey_time;
			}

			if ( apmib_set(MIB_WLAN_MESH_PSK_FORMAT, (void *)&intVal) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_MESH_PSK_FORMAT failed!"));
				goto setErr_mEncrypt;
			}

			if (intVal==1) { // hex
				if (len!=MAX_PSK_LEN || !string_to_hex(strVal, tmpBuf, MAX_PSK_LEN)) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_mEncrypt;
				}
			}
			else { // passphras
				if (len==0 || len > (MAX_PSK_LEN-1) ) {
	 				strcpy(tmpBuf, ("Error! invalid psk value."));
					goto setErr_mEncrypt;
				}
			}
			if ( !apmib_set(MIB_WLAN_MESH_WPA_PSK, (void *)strVal)) {
				strcpy(tmpBuf, ("Set MIB_WLAN_MESH_WPA_PSK error!"));
				goto setErr_mEncrypt;
			}
		}	
	}
mRekey_time:
		// group key rekey time			
	return 0 ;
setErr_mEncrypt:
	return -1 ;		
}	

#ifdef 	_11s_TEST_MODE_
void formEngineeringMode(char *postData, int length)
{
	char *submitUrl;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
	char *param;
	int val;
	//
	param = get_cstream_var(postData, length, "param1", "");
			
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM1, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved1=%d", val);
	system(tmpBuf);
	
	param = get_cstream_var(postData, length, "param2", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM2, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved2=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param3", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM3, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved3=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param4", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM4, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved4=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param5", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM5, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved5=%d", val);
	system(tmpBuf);
	
	param = get_cstream_var(postData, length, "param6", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM6, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved6=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param7", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM7, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved7=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param8", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM8, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved8=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "param9", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAM9, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserved9=%d", val);
	system(tmpBuf);
	
	param = get_cstream_var(postData, length, "parama", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMA, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reserveda=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "paramb", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMB, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedb=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "paramc", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMC, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedc=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "paramd", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMD, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedd=%d", val);
	system(tmpBuf);
	
	param = get_cstream_var(postData, length, "parame", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAME, (void *)&val)==0 )
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservede=%d", val);
	system(tmpBuf);

	param = get_cstream_var(postData, length, "paramf", "");
	string_to_dec(param , &val);
	if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMF, (void *)&val)==0 )	
		goto setErr_meshTest;
	sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedf=%d", val);
	system(tmpBuf);
	
	param = get_cstream_var(postData, length, "paramstr1", "");
    if (param[0])
    {
            if (strlen(param)>16) 
                  goto setErr_meshTest;

            if ( apmib_set(MIB_WLAN_MESH_TEST_PARAMSTR1, (void *)param) == 0)
                    goto setErr_meshTest;
			sprintf(tmpBuf, "iwpriv wlan0 set_mib mesh_reservedstr1='%s'", param);
			system(tmpBuf);			
    }
    apmib_update(CURRENT_SETTING);
/*
#ifndef NO_ACTION
        run_init_script("bridge");
#endif
*/
	submitUrl = get_cstream_var(postData, length, ("meshtest-url"), "");   // hidden page
	OK_MSG(submitUrl);
	return;

setErr_meshTest:
		strcpy(tmpBuf, ("Error! set Mesh Test Param Error!!! "));
        ERR_MSG(tmpBuf);	 
}

void formEngineeringMode2(char *postData, int length)
{
	char *submitUrl;
	char	*strCMD;
	char tmpBuf[200];
	strCMD = get_cstream_var(postData, length, ("cmd"), "");
	system(strCMD);
	submitUrl = get_cstream_var(postData, length, ("meshtest-url"), "");   // hidden page
	OK_MSG1(tmpBuf, submitUrl);
}

#endif



#ifdef _MESH_ACL_ENABLE_
int wlMeshAcList(char *postData, int length)
{
	int nBytesSent=0, entryNum, i;
	MACFILTER_T entry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	if ( !apmib_get(MIB_WLAN_MESH_ACL_NUM, (void *)&entryNum)) {
  		fprintf(stderr, "Get MIB_WLAN_MESH_ACL_NUM table entry error!\n");
		return -1;
	}

	nBytesSent += web_write_chunked(("<tr>"
      	"<td align=center width=\"45%%\" bgcolor=\"#808080\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
      	"<td align=center width=\"35%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Comment</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));

	for (i=1; i<=entryNum; i++) {
		*((char *)&entry) = (char)i;
		if ( !apmib_get(MIB_WLAN_MESH_ACL_ADDR, (void *)&entry))
			return -1;

		snprintf(tmpBuf, MAX_MSG_BUFFER_SIZE, ("%02x:%02x:%02x:%02x:%02x:%02x"),
			entry.macAddr[0], entry.macAddr[1], entry.macAddr[2],
			entry.macAddr[3], entry.macAddr[4], entry.macAddr[5]);

		nBytesSent += web_write_chunked(("<tr>"
			"<td align=center width=\"45%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				tmpBuf, entry.comment, i);
	}
	return nBytesSent;
}

void formMeshACLSetup(char *postData, int length)
{
	char *submitUrl;
	char *strAddMac, *strDelMac, *strDelAllMac, *strVal, *strEnabled;
	int entryNum, i, enabled;
	MACFILTER_T macEntry;
	char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};

	strAddMac = get_cstream_var(postData, length, ("addMeshAclMac"), "");
	strDelMac = get_cstream_var(postData, length, ("deleteSelMeshAclMac"), "");
	strDelAllMac = get_cstream_var(postData, length, ("deleteAllMeshAclMac"), "");
	strEnabled = get_cstream_var(postData, length, ("meshAclEnabled"), "");
	submitUrl = get_cstream_var(postData, length, ("mesh-url"), "");   // hidden page

	if (strAddMac[0]) {
		/*if ( !strcmp(strEnabled, "ON"))
			enabled = 1;
		else
			enabled = 0; */ //by sc_yang
		 enabled = strEnabled[0] - '0';
		if ( apmib_set( MIB_WLAN_MESH_ACL_ENABLED, (void *)&enabled) == 0) {
  			strcpy(tmpBuf, ("Set enabled flag error!"));
			goto setErr_meshACL;
		}

		strVal = get_cstream_var(postData, length, ("aclmac"), "");
		if ( !strVal[0] ) {		// For Disable/Allow/Deny mode setting.
//			strcpy(tmpBuf, ("Error! No mac address to set."));
			goto meshAclExit;
		}
		if (strlen(strVal)!=12 || !string_to_hex(strVal, macEntry.macAddr, 12)) {
			strcpy(tmpBuf, ("Error! Invalid MAC address."));
			goto setErr_meshACL;
		}

		strVal = get_cstream_var(postData, length, ("aclcomment"), "");
		if ( strVal[0] ) {
			if (strlen(strVal) > COMMENT_LEN-1) {
				strcpy(tmpBuf, ("Error! Comment length too long."));
				goto setErr_meshACL;
			}
			strcpy(macEntry.comment, strVal);
		}
		else
			macEntry.comment[0] = '\0';

		if ( !apmib_get(MIB_WLAN_MESH_ACL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_meshACL;
		}
		if ( (entryNum + 1) > MAX_MESH_ACL_NUM) {
			strcpy(tmpBuf, ("Cannot add new entry, Because table is full!"));
			goto setErr_meshACL;
		}
		// set to MIB. try to delete it first to avoid duplicate case
		apmib_set(MIB_WLAN_MESH_ACL_ADDR_DEL, (void *)&macEntry);
		if ( apmib_set(MIB_WLAN_MESH_ACL_ADDR_ADD, (void *)&macEntry) == 0) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_meshACL;
		}
		goto meshAclExit;
	}

	/* Delete entry */
	if (strDelMac[0]) {
		if ( !apmib_get(MIB_WLAN_MESH_ACL_NUM, (void *)&entryNum)) {
			strcpy(tmpBuf, ("Get entry number error!"));
			goto setErr_meshACL;
		}
		for (i=entryNum; i>0; i--) {
			snprintf(tmpBuf, 20, "select%d", i);

			strVal = get_cstream_var(postData, length, tmpBuf, "");
			if ( !strcmp(strVal, "ON") ) {

				*((char *)&macEntry) = (char)i;
				if ( !apmib_get(MIB_WLAN_MESH_ACL_ADDR, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Get table entry error!"));
					goto setErr_meshACL;
				}
				if ( !apmib_set(MIB_WLAN_MESH_ACL_ADDR_DEL, (void *)&macEntry)) {
					strcpy(tmpBuf, ("Delete table entry error!"));
					goto setErr_meshACL;
				}
			}
		}
		goto meshAclExit;
	}

	/* Delete all entry */
	if ( strDelAllMac[0]) {
		if ( !apmib_set(MIB_WLAN_MESH_ACL_ADDR_DELALL, (void *)&macEntry)) {
			strcpy(tmpBuf, ("Delete all table error!"));
			goto setErr_meshACL;
		}
		goto meshAclExit;
	}

meshAclExit:
#ifndef NO_ACTION
        run_init_script("bridge");
#endif
        apmib_update(CURRENT_SETTING);

        submitUrl = get_cstream_var(postData, length, ("mesh-url"), "");   // hidden page
//#ifdef REBOOT_CHECK
        OK_MSG(submitUrl);
//#else
#if 0
	RECONNECT_MSG(submitUrl);       // display reconnect msg to remote
#endif
//#endif

        return;

setErr_meshACL:
        ERR_MSG(tmpBuf);
}
#endif	// _MESH_ACL_ENABLE_

int formMeshProxyTbl(char *postData, int length)
{
        char *submitUrl,*refresh;

        submitUrl = get_cstream_var(postData, length, ("mesh-url"), "");   // hidden page
        refresh = get_cstream_var(postData, length, ("refresh"), "");

        if ( refresh[0] )
        {
                get_cstream_var(postData, length, submitUrl,"");
                return;
        }
}
char * _get_token( FILE * fPtr,char * token,char * data )
{
        char buf[512];
        char * pch;

        strcpy( data,"");

        if( fgets(buf, sizeof buf, fPtr) == NULL ) // get a new line
                return NULL;

        pch = strstr( buf, token ); //parse the tag

        if( pch == NULL )
                return NULL;

        pch += strlen( token );

        sprintf( data,"%s",pch );                  // set data

        return pch;
}


void strtolower(char *str, int len)
{
	int i;
	for (i = 0; i<len; i++) {
		str[i] = tolower(str[i]);
	}
}


void formMeshProxy(char *postData, int length)
{
	char *strPrxyOwnr;
	int nRecordCount=0;
	FILE *fh;
	char buf[512];
	unsigned char sta[20],owner[20], macstr[20];
	
	strPrxyOwnr = get_cstream_var(postData, length, ("owner"), "");
	strtolower(strPrxyOwnr, 12);
	
	// show proxy
	if ( strPrxyOwnr[0] )
	{
		sprintf(macstr, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", strPrxyOwnr[0],strPrxyOwnr[1],strPrxyOwnr[2]
			,strPrxyOwnr[3],strPrxyOwnr[4],strPrxyOwnr[5],strPrxyOwnr[6],strPrxyOwnr[7],strPrxyOwnr[8]
			,strPrxyOwnr[9],strPrxyOwnr[10],strPrxyOwnr[11]);
		web_write_chunked(("<html><! Copyright (c) Realtek Semiconductor Corp., 2003~2005. All Rights Reserved. ->\n"));
		web_write_chunked(("<head><meta http-equiv=\"Content-Type\" content=\"text/html\">\n"));
		//req_format_write(wp, ("<script type=\"text/javascript\" src=\"util_gw.js\"></script>\n"));
		web_write_chunked(("<title>Proxy Table</title></head>\n"));
		web_write_chunked(("<blockquote><h2><font color=\"#0000FF\">Active Client Table - %s</font></h2>\n"), macstr);
		web_write_chunked(("<body><form action=/boafrm/formMeshProxy method=POST name=\"formMeshProxy\">\n"));
		web_write_chunked(("<table border=0 width=550 cellspacing=4 cellpadding=0>\n"));
		web_write_chunked(("<tr><font size=2>\n"));
		web_write_chunked(("This table shows the MAC address for each proxied wired or wireless client\n"));
		web_write_chunked(("</font></tr>\n"));
		web_write_chunked(("<tr><hr size=1 noshade align=top></tr></table>\n"));
		
		
		web_write_chunked(("<table border=1 width=200>\n"));
		//req_format_write(wp, ("<tr><font size=4><b>Proxy Table </b></font></tr>\n"));
		
				
		web_write_chunked(("<tr bgcolor=\"#7F7F7F\">"
		//"<td align=center width=\"50%%\"><font size=\"2\"><b>MP MAC Address</b></font></td>\n"
		"<td align=center><font size=\"2\"><b>Client MAC Address</b></font></td></tr>\n"));
		
		sprintf(buf,"/proc/wlan%d/%s",apmib_get_wlanidx(),_FILE_MESH_PROXY);
        fh = fopen(buf, "r");
		if (!fh)
		{
				printf("Warning: cannot open %s\n",buf );
				return -1;
		}
		
		while( fgets(buf, sizeof buf, fh) != NULL )
		{
			if( strstr(buf,"table info...") != NULL )
			{
				_get_token( fh,"STA_MAC: ",sta );
				_get_token( fh,"OWNER_MAC: ",owner );
				strtolower(owner, 12);
				if (!strncmp(strPrxyOwnr,owner,12)){
					web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
							"<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"),sta);
					nRecordCount++;
				}
			}
		}
		
		fclose(fh);
		
		if(nRecordCount == 0)
		{
			web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
					"<td align=center width=\"17%%\"><font size=\"2\">None</td>\n"));
		}
				
		web_write_chunked(("</tr></table>\n"));
		web_write_chunked(("<input type=\"hidden\" value=\"%s\" name=\"owner\">\n"), strPrxyOwnr);
		web_write_chunked(("<p><input type=\"submit\" value=\"Refresh\" name=\"refresh\">&nbsp;&nbsp;\n"));
		web_write_chunked(("<input type=\"button\" value=\" Close \" name=\"close\" onClick=\"javascript: window.close();\"><p>\n"));
		web_write_chunked(("</form>\n"));
		web_write_chunked(("</blockquote></body></html>"));
	}
}

void formMeshSetup(char *postData, int length)
{	
    char *submitUrl,*meshRootEnabled,*refresh, *strMeshID, *strEnabled;
    int enabled,meshenable=0;
    char tmpBuf[MAX_MSG_BUFFER_SIZE]={0};
    int warn=0;
	
#ifdef CONFIG_NEW_MESH_UI
	#if 1
	meshRootEnabled = get_cstream_var(postData, length, ("meshRootEnabled"), "");
	#else
	meshRootEnabled = "ON";
	#endif
#else
    meshRootEnabled = get_cstream_var(postData, length, ("meshRootEnabled"), "");
#endif
    strMeshID = get_cstream_var(postData, length, ("meshID"), "");
    submitUrl = get_cstream_var(postData, length, ("mesh-url"), "");   // hidden page
    refresh = get_cstream_var(postData, length, ("refresh"), "");
	//new feature:Mesh enable/disable
	strEnabled = get_cstream_var(postData, length, ("wlanMeshEnable"), "");
	
	// refresh button response
    if ( refresh[0] )
    {
    		send_redirect_perm(submitUrl);
            return;
    }
	
	if ( !strcmp(strEnabled, "ON"))
		meshenable = 1;
	else
		meshenable = 0;

	if ( apmib_set(MIB_WLAN_MESH_ENABLE, (void *)&meshenable) == 0)
    {
            strcpy( tmpBuf, ("Set mesh enable error!"));
            goto setErr_mesh;
    }

	if( !meshenable )
		goto setupEnd;

	// backbone privacy settings
	if(meshWpaHandler(postData, MAX_MSG_BUFFER_SIZE,apmib_get_wlanidx()) < 0)
		goto setErr_mesh;
		
#ifdef CONFIG_NEW_MESH_UI
if(!strcmp(meshRootEnabled, "ON"))
        enabled = 1 ;
else
        enabled = 0 ;
#else
    if(!strcmp(meshRootEnabled, "ON"))
            enabled = 1 ;
    else
            enabled = 0 ;
#endif
    if ( apmib_set(MIB_WLAN_MESH_ROOT_ENABLE, (void *)&enabled) == 0)
    {
            strcpy( tmpBuf, ("Set mesh Root enable error!"));
            goto setErr_mesh;
    }

    if (strMeshID[0])
    {
//              if (strlen(strMeshID)!=12 || !string_to_hex(strMeshID, tmpBuf, 12)) {
            if (strlen(strMeshID)>32) {
                    strcpy(tmpBuf, ("Error! Invalid Mesh ID."));
                    goto setErr_mesh;
            }
            if ( apmib_set(MIB_WLAN_MESH_ID, (void *)strMeshID) == 0)
            {
                    strcpy(tmpBuf, ("Set MIB_WLAN_MESH_ID error!"));
                    goto setErr_mesh;
            }
    }
setupEnd:
    apmib_update(CURRENT_SETTING);

#ifndef NO_ACTION
    run_init_script("bridge");
#endif

	if (warn) {
          OK_MSG1(tmpBuf, submitUrl);
    }
    else {
	OK_MSG(submitUrl);
    }
    return;

setErr_mesh:
    ERR_MSG(tmpBuf);
}

int wlMeshNeighborTable(char *postData, int len)
{
  	int nBytesSent=0;
    int nRecordCount=0;
    FILE *fh;
	char interf[15];
	unsigned char * macaddr;
	
	#define MESH_NEIGHBOR_TABLE_INFO_MAX_NUM 10
	struct _MESH_NEIGHBOR_TABLE_INFO{
		unsigned char  	macAddr[6];
		unsigned char 	mode[10];
		unsigned short 	txRate;
		unsigned short 	rssi;
		unsigned int 	txPkt;
		unsigned int 	rxPkt;
		unsigned long 	expir;
	}; 
	struct _MESH_NEIGHBOR_TABLE_INFO mesh_neighbor_table_info[MESH_NEIGHBOR_TABLE_INFO_MAX_NUM];
   	memset(mesh_neighbor_table_info,0,sizeof(mesh_neighbor_table_info));
		
    nBytesSent += web_write_chunked(("<tr bgcolor=\"#7F7F7F\">"
    "<td align=center width=\"17%%\"><font size=\"2\"><b>MAC Address</b></font></td>\n"
    //"<td align=center width=\"17%%\"><font size=\"2\"><b>State</b></font></td>\n"
    "<td align=center width=\"17%%\"><font size=\"2\"><b>Mode</b></font></td>\n"
    //"<td align=center width=\"17%%\"><font size=\"2\"><b>Channel</b></font></td>\n"
    "<td align=center width=\"17%%\"><font size=\"2\"><b>Tx Packets</b></font></td>\n"
	"<td align=center width=\"17%%\"><font size=\"2\"><b>Rx Packets</b></font></td>\n"
    "<td align=center width=\"17%%\"><font size=\"2\"><b>Tx Rate (Mbps)</b></font></td>\n"
    "<td align=center width=\"17%%\"><font size=\"2\"><b>RSSI</b></font></td>\n"
	"<td align=center width=\"17%%\"><font size=\"2\"><b>Expired Time (s)</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
    "<td align=center width=\"17%%\"><font size=\"2\"><b>BootSeq_ept</b></font></td></tr>\n"
#endif
	));

	sprintf(interf,"wlan%d",apmib_get_wlanidx());
	if(!getWlMeshNeighborTableInfo (interf, mesh_neighbor_table_info,sizeof(mesh_neighbor_table_info)))
	{	
		macaddr = mesh_neighbor_table_info[nRecordCount].macAddr;
		while(macaddr[0] || macaddr[1] || macaddr[2] || macaddr[3] || macaddr[4] || macaddr[5])
		{
	       nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
	        "<td align=center width=\"17%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"
	        "<td align=center width=\"17%%\"><font size=\"2\">%s</td>\n"
	        "<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"
	        "<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"
#if defined(_11s_TEST_MODE_)
			"<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"
	        "<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"),
	        //hwaddr,state,channel,link_rate,rssi,establish_exp_time,bootseq_exp_time);
	        macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5],mesh_neighbor_table_info[nRecordCount].mode,mesh_neighbor_table_info[nRecordCount].txPkt,mesh_neighbor_table_info[nRecordCount].rxPkt,mesh_neighbor_table_info[nRecordCount].txRate,mesh_neighbor_table_info[nRecordCount].rssi,mesh_neighbor_table_info[nRecordCount].expir,mesh_neighbor_table_info[nRecordCount].expir);
#else
			"<td align=center width=\"17%%\"><font size=\"2\">%d</td>\n"),
			//hwaddr,state,channel,link_rate,rssi);
			 macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5],mesh_neighbor_table_info[nRecordCount].mode,mesh_neighbor_table_info[nRecordCount].txPkt,mesh_neighbor_table_info[nRecordCount].rxPkt,mesh_neighbor_table_info[nRecordCount].txRate,mesh_neighbor_table_info[nRecordCount].rssi,mesh_neighbor_table_info[nRecordCount].expir);
#endif
			
	        nRecordCount++;
			if(nRecordCount > MESH_NEIGHBOR_TABLE_INFO_MAX_NUM)
				break;
			macaddr = mesh_neighbor_table_info[nRecordCount].macAddr;

		}
	}

	if(nRecordCount == 0)
	{
	 	nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
        "<td align=center><font size=\"2\">None</td>"
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"                        
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
		"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
		"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#if defined(_11s_TEST_MODE_)
		"<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#endif
		));
}
    return nBytesSent;
}

/////////////////////////////////////////////////////////////////////////////
int wlMeshRoutingTable(char *postData, int len)
{
	int nBytesSent=0;
    int nRecordCount=0;
	char interf[15];
	unsigned char *dmac;
	unsigned char *nmac;
	
	#define MESH_ROUTE_TABLE_INFO_MAX_NUM 10
	struct _MESH_ROUTE_TABLE_INFO{
	unsigned char  	dMacAddr[6];
	unsigned char 	nextMacAddr[6];
	unsigned char 	portalEnable[5];
	unsigned char 	hopCount;
	unsigned int 	metric;
	unsigned int 	start;
	unsigned int 	end;
	unsigned int 	diff;
	}; 
	struct _MESH_ROUTE_TABLE_INFO mesh_route_table_info[MESH_ROUTE_TABLE_INFO_MAX_NUM];
	memset(mesh_route_table_info,0,sizeof(mesh_route_table_info));
	
    nBytesSent += web_write_chunked( ("<tr bgcolor=\"#7F7F7F\">"
    "<td align=center width=\"15%%\"><font size=\"2\"><b>Destination Mesh Point</b></font></td>\n"
    "<td align=center width=\"15%%\"><font size=\"2\"><b>Next-hop Mesh Point</b></font></td>\n"
"<td align=center width=\"10%%\"><font size=\"2\"><b>Portal Enable</b></font></td>\n"
    //"<td align=center width=\"10%%\"><font size=\"2\"><b>DSN</b></font></td>\n"
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Metric</b></font></td>\n"
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Hop Count</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Gen PREQ</b></font></td>\n"
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Rev PREP</b></font></td>\n"
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Delay</b></font></td>\n"
    "<td align=center width=\"10%%\"><font size=\"2\"><b>Flag</b></font></td></tr>\n"
#endif
	));

	nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
			"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
#if defined(_11s_TEST_MODE_)
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
#endif
			  ),"My-self","---","yes","---","---"
#if defined(_11s_TEST_MODE_)
			, "---", "---", "---", "---"
#endif
			);

	sprintf(interf,"wlan%d",apmib_get_wlanidx());
	if(!getWlMeshRouteTableInfo (interf, mesh_route_table_info,sizeof(mesh_route_table_info)))
	{	
		dmac = mesh_route_table_info[0].dMacAddr;
		while(dmac[0] || dmac[1] || dmac[2] || dmac[3] || dmac[4] || dmac[5])
		{
			nmac = mesh_route_table_info[nRecordCount].nextMacAddr;
			nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
			"<td align=center width=\"15%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"
			"<td align=center width=\"15%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"
			"<td align=center width=\"15%%\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%d</td>\n"
#if defined(_11s_TEST_MODE_)
			"<td align=center width=\"10%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%d</td>\n"
			"<td align=center width=\"10%%\"><font size=\"2\">%s</td>\n"
#endif
			  ),dmac[0],dmac[1],dmac[2],dmac[3],dmac[4],dmac[5],nmac[0],nmac[1],nmac[2],nmac[3],nmac[4],nmac[5],\
			 mesh_route_table_info[nRecordCount].portalEnable,mesh_route_table_info[nRecordCount].metric,mesh_route_table_info[nRecordCount].hopCount
#if defined(_11s_TEST_MODE_)
			, mesh_route_table_info[nRecordCount].start, mesh_route_table_info[nRecordCount].end, mesh_route_table_info[nRecordCount].diff, "---"
#endif
			);
			nRecordCount++;
			if(nRecordCount > MESH_ROUTE_TABLE_INFO_MAX_NUM)
				break;
			dmac = mesh_route_table_info[nRecordCount].dMacAddr;
		}
	}
    if(nRecordCount == 0)
    {
	    nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
        "<td><font size=\"2\">None</td>"
        "<td align=center width=\"15%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"15%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"                        
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
#if defined(_11s_TEST_MODE_)
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\">---</td>\n"
#endif
		"<font size=\"2\">---</td>\n"
		));
    } 
					
        return nBytesSent;
}

int wlMeshPortalTable(char *postData, int len)
{
	int nBytesSent=0;
    int nRecordCount=0;
    char interf[15];
	unsigned char* mac;
	
	#define MESH_PORTAL_TABLE_INFO_MAX_NUM 10
	struct _MESH_PORTAL_TABLE_INFO{
	unsigned char  	macAddr[6];
	unsigned int 	timeout;
	unsigned int 	seqNum;
	}; 
	struct _MESH_PORTAL_TABLE_INFO mesh_portal_table_info[MESH_PORTAL_TABLE_INFO_MAX_NUM];
	memset(mesh_portal_table_info,0,sizeof(mesh_portal_table_info));

	nBytesSent += web_write_chunked(("<tr bgcolor=\"#7F7F7F\">"
    "<td align=center width=\"16%%\"><font size=\"2\"><b>PortalMAC</b></font></td>\n"
#if defined(_11s_TEST_MODE_)
    "<td align=center width=\"16%%\"><font size=\"2\"><b>timeout</b></font></td>\n"
    "<td align=center width=\"16%%\"><font size=\"2\"><b>seqNum</b></font></td></tr>\n"
#endif
	));

	sprintf(interf,"wlan%d",apmib_get_wlanidx());
	if(!getWlMeshPortalTableInfo (interf, mesh_portal_table_info,sizeof(mesh_portal_table_info)))
	{	
		mac = mesh_portal_table_info[0].macAddr;
		while(mac[0] || mac[1] || mac[2] || mac[3] || mac[4] || mac[5])
		{
			 nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
            "<td align=center width=\"16%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"
#if defined(_11s_TEST_MODE_)
            "<td align=center width=\"16%%\"><font size=\"2\">%s</td>\n"
            "<td align=center width=\"16%%\"><font size=\"2\">%s</td>\n"
#endif
    		), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
#if defined(_11s_TEST_MODE_)
			,mesh_portal_table_info[nRecordCount].timeout, mesh_portal_table_info[nRecordCount].seqNum
#endif
			);
			nRecordCount++;
			if(nRecordCount > MESH_PORTAL_TABLE_INFO_MAX_NUM)
				break;
			mac = mesh_portal_table_info[nRecordCount].macAddr;
		}
	}
    if(nRecordCount == 0)
    {
	    nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
	    "<td><font size=\"2\">None</td>"
#if defined(_11s_TEST_MODE_)
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"
#endif
		));
    }
    return nBytesSent;
}


/////////////////////////////////////////////////////////////////////////////
int wlMeshProxyTable(char *postData, int len)
{
    int nBytesSent=0;
    int nRecordCount=0;
	char interf[15];
	unsigned char* smac;
	unsigned char* omac;
	
	#define MESH_PROXY_TABLE_INFO_MAX_NUM 10
	struct _MESH_PROXY_TABLE_INFO{
	unsigned char  	staMac[6];
	unsigned char  	ownerMac[6];
	}; 
	struct _MESH_PROXY_TABLE_INFO mesh_proxy_table_info[MESH_PROXY_TABLE_INFO_MAX_NUM];
	memset(mesh_proxy_table_info,0,sizeof(mesh_proxy_table_info));
	
    nBytesSent += web_write_chunked( ("<tr bgcolor=\"#7F7F7F\">"
    "<td align=center width=\"50%%\"><font size=\"2\"><b>Owner</b></font></td>\n"
    "<td align=center width=\"50%%\"><font size=\"2\"><b>Client</b></font></td></tr>\n"));

	sprintf(interf,"wlan%d",apmib_get_wlanidx());
	if(!getWlMeshProxyTableInfo (interf, mesh_proxy_table_info,sizeof(mesh_proxy_table_info)))
	{	
		smac = mesh_proxy_table_info[0].staMac;
		omac = mesh_proxy_table_info[0].ownerMac;
		while(smac[0] || smac[1] || smac[2] || smac[3] || smac[4] || smac[5])
		{
			nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
            "<td align=center width=\"50%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"
            "<td align=center width=\"50%%\"><font size=\"2\">%02x%02x%02x%02x%02x%02x</td>\n"),
            omac[0], omac[1], omac[2], omac[3], omac[4], omac[5],smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]);
            nRecordCount++;
			if(nRecordCount > MESH_PROXY_TABLE_INFO_MAX_NUM)
				break;
			smac = mesh_proxy_table_info[nRecordCount].staMac;
			omac = mesh_proxy_table_info[nRecordCount].ownerMac;
		}
	}

    if(nRecordCount == 0)
    {
        nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
        "<td><font size=\"2\">None</td>"
        "<td align=center width=\"17%%\"><font size=\"2\">---</td>\n"));
    }
    return nBytesSent;
}

#ifdef _11s_TEST_MODE_
int wlRxStatics(char *postData, int len)
{
        int nBytesSent=0;
        FILE *fh;
        char buf[512];
        char buf2[15][50];

        nBytesSent += web_write_chunked( ("<tr bgcolor=\"#7F7F7F\">"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>jiffies</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_packets</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_retrys</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_errors</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>rx_packets</b></font></td>\n"
        "<td align=center width=\"10%%\"><font size=\"2\"><b>tx_pkts</b></font></td>\n"
		"<td align=center width=\"10%%\"><font size=\"2\"><b>rx_pkts</b></font></td>\n"         
        "<td align=center width=\"10%%\"><font size=\"2\"><b>rx_crc_errors</b></font></td>\n"));


		sprintf(buf,"/proc/wlan%d/%s",apmib_get_wlanidx(),_FILE_MESHSTATS);
        fh = fopen(buf, "r");
        if (!fh)
        {
                printf("Warning: cannot open %s\n",buf );
                return -1;
        }

		if( fgets(buf, sizeof buf, fh) && strstr(buf,"Statistics..."))
        {
				_get_token( fh,"OPMODE: ", buf2[0] );
				_get_token( fh,"jiffies: ",buf2[1] );
		}
        if( fgets(buf, sizeof buf, fh) && strstr(buf,"Statistics..."))
        {
				_get_token( fh,"tx_packets: ",buf2[2] );
				_get_token( fh,"tx_bytes: ",buf2[3] );
                _get_token( fh,"tx_errors: ",buf2[4] );

				_get_token( fh,"rx_packets: ",buf2[5] );
				_get_token( fh,"rx_bytes: ",buf2[6] );
				_get_token( fh,"rx_errors: ",buf2[7] );				
                _get_token( fh,"rx_crc_errors: ",buf2[8] );

                nBytesSent += web_write_chunked(("<tr bgcolor=\"#b7b7b7\">"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"                                     
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"
                        "<td align=center width=\"50%%\"><font size=\"2\">%s</td>\n"),
                        buf2[1], buf2[2], buf2[3], buf2[4], buf2[5], buf2[6], buf2[7], buf2[8] ); 
        }

        fclose(fh);

        return nBytesSent;
}
#endif

int wlMeshRootInfo(char *postData, int len)
{
    int nBytesSent=0;
    unsigned char rootmac[6];
	char interf[15];
	memset(rootmac,0,sizeof(rootmac));
	
	strcpy(interf, "wlan-msh0"); 
	if(!getwlMeshRootInfo (interf, rootmac,sizeof(rootmac)))
	{
		if(rootmac[0]||rootmac[1]||rootmac[2]||rootmac[3]||rootmac[4]||rootmac[5])
	         nBytesSent += web_write_chunked("%02x%02x%02x%02x%02x%02x", rootmac[0],rootmac[1],rootmac[2],rootmac[3],rootmac[4],rootmac[5]);
		else
		     nBytesSent += web_write_chunked(("None"));
	}
	else
		 nBytesSent += web_write_chunked(("None"));
    return nBytesSent;
}


#endif // CONFIG_RTK_MESH

/*************************************************************************/
#ifdef HAVE_WLAN_SCHEDULE
void formSchedule(char *postData, int length)
{
	char	tmpBuf[100];
	char urlbuf[60];
	char *strHours, *strEnabled, *strWeekdays, *strStime, *strEtime;
	SCHEDULE_T entry;
	int entryNum=0;
	char *submitUrl;
	int isEnabled=0;
	submitUrl = get_cstream_var(postData,length, ("webpage"), "");   // hidden page
	sprintf(urlbuf,"/formRedirect.htm?redirect-url=%s&wlan_id=%d",submitUrl+1,apmib_get_wlanidx());
	
	if ( !apmib_set(MIB_WLAN_SCHEDULE_DELALL, (void *)&entry)) {
			strcpy(tmpBuf, ("Delete table entry error!"));
			goto setErr_schedule;
	}
	memset(&entry, '\0', sizeof(entry));
	
	strEnabled = get_cstream_var(postData,length, ("enabled_sch"), "");
	if(strcmp(strEnabled,"true") == 0) // the entry is enabled
	{
			entry.eco |= ECO_LEDDIM_MASK;
			isEnabled = 1;
	}else{
			entry.eco &= ~ECO_LEDDIM_MASK;
			isEnabled = 0;
	}
	apmib_set(MIB_WLAN_SCHEDULE_ENABLED,(void *)&isEnabled);
	sprintf((char *)entry.text, "%s", "wlanSchedule");	
	
	strWeekdays = get_cstream_var(postData,length, ("weekdays"), "");
	entry.day = atoi(strWeekdays);

	

	if(strcmp(strWeekdays, "127") ==0)
	{
		entry.eco |= ECO_EVERYDAY_MASK;
	}else
		entry.eco &= ~ECO_EVERYDAY_MASK;
		  
	strHours = get_cstream_var(postData,length, ("all_day"), "");	

	if(strcmp(strHours,"on") == 0) // the entry is enabled 24 hours
	{
		entry.eco |= ECO_24HOURS_MASK;
		
	}else
		entry.eco &= ~ECO_24HOURS_MASK;

	strStime = get_cstream_var(postData,length, ("start_time"), "");
	if(strStime[0])
		entry.fTime = atoi(strStime);

	strEtime = get_cstream_var(postData,length, ("end_time"), "");
	if(strEtime[0])
		entry.tTime = atoi(strEtime);
	
	if(entry.eco & ECO_24HOURS_MASK){
			entry.fTime = 0;
			entry.tTime = 1440;
	}
	
	if ( !apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)) 
	{
			strcpy(tmpBuf, ("\"Get entry number error!\""));
			goto setErr_schedule;
	}
	if ( !apmib_set(MIB_WLAN_SCHEDULE_ADD,(void *)&entry)) {
			strcpy(tmpBuf, ("Add table entry error!"));
			goto setErr_schedule;
	}
	
	
	apmib_update_web(CURRENT_SETTING);
#ifndef NO_ACTION
//	run_init_script("bridge");
#endif
	//kick_event(WLAN_APP_EVENT);
	//extern 	void start_wlanSchedule();

#ifdef HAVE_WLAN_SCHEDULE
	start_wlanSchedule(0);
#endif
#ifdef HAVE_SYSTEM_REINIT
		{
			wait_redirect("Apply Changes",2,urlbuf);
			sleep(1);
			//kick_reinit_m(SYS_WIFI_APP_M);
		}
#elif defined(WIFI_QUICK_REINIT)
		if(MUST_REBOOT != 1)
		{
			kick_event(WLAN_BRIDGE_EVENT);
			send_redirect_perm(submitUrl);
		}
		else
			OK_MSG(submitUrl);
#else
		OK_MSG(submitUrl);
#endif

	save_cs_to_file();

	return;

setErr_schedule:
	ERR_MSG(urlbuf);
	
}
#endif
/****************************************************************************/
void formWirelessTbl(char *postData, int length)
{
	char *submitUrl;

	submitUrl = get_cstream_var(postData,length, ("submit-url"), "");
	if (submitUrl[0])
		send_redirect_perm(submitUrl);
}

/****************************************************************************/
#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
void enable_repeater_underlying(char *postData, int length)
{
	char *str_opmode;
	int val;
	str_opmode = get_cstream_var(postData,length, ("opmode"), "");
	if (str_opmode[0]) {
		if (WISP_MODE == atoi(str_opmode)) {	
			val = 1;
			apmib_set(MIB_REPEATER_ENABLED1, (void *)&val);
			apmib_set(MIB_REPEATER_ENABLED2, (void *)&val); 	
		}
	}
}
#endif /* #if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) */

/****************************************************************************/
#if defined(BRIDGE_REPEATER) && defined(DHCP_AUTO_SUPPORT) && defined(CONFIG_RTL_ULINKER)
SITE_SURVEY_STATUS siteSurveyStatus = SITE_SURVEY_STATUS_OFF;
#endif
void formWlSiteSurvey(char *postData, int length)
{
 	char *submitUrl, *refresh, *connect, *strSel, *strVal;
	int status, idx, encrypt;
	BssDscr *pBss=NULL;
	char ssidbuf[33];
	int k;
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
	int wpaPSK;	// For wpa/wpa2
#endif
	unsigned char res, *pMsg=NULL;
	int wait_time, max_wait_time=5;
	char tmpBuf[100];
#if defined(WLAN_PROFILE)
	int profile_enabled_id, profileEnabledVal, oriProfileEnabledVal;
#endif
	int isWizard=0;
	int vxd_wisp_wan=0;
	int client_wisp_wan=0;
	char wlanifp_bak[32]={0};
	char *wlanifp=NULL;
	int wlan_disabled=0;
	int connect_timeout=10;


#if (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && !defined(CONFIG_CUTE_MAHJONG_RTK_UI))
	enable_repeater_underlying(postData, length);
#endif

#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
	int mesh_enable=0; 
	
// fixed by Joule 2009.01.10
	if(apmib_get(MIB_WLAN_MODE, (void *)&mesh_enable) == 0 || mesh_enable < 4) 
		mesh_enable = 0; 	
#endif 
	submitUrl = get_cstream_var(postData,length, ("submit-url"), "");

	refresh = get_cstream_var(postData,length, ("refresh"), "");
	if ( refresh[0] ) {
		// issue scan request
		if(apmib_get(MIB_WLAN_WLAN_DISABLED,&wlan_disabled)==0)
		{
			strcpy(tmpBuf, ("get mib value fail!")); 
			goto ss_err_end;
		}
		if(wlan_disabled)
		{
			strcpy(tmpBuf, ("wlan is disabled!")); 
			goto ss_err_end;
		}
		wait_time = 0;
		while (1) {
			strVal = get_cstream_var(postData,length, ("ifname"), "");
			if(strVal[0])
			{
				sprintf(WLAN_IF,"%s",strVal);				
			}
			 
			// ==== modified by GANTOE for site survey 2008/12/26 ==== 
			switch(getWlSiteSurveyRequest(WLAN_IF, &status)) 
			{ 
				case -2: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Auto scan running!!please wait...")); 
					goto ss_err_end; 
					break; 
				case -1: 
					printf("-2\n"); 
					strcpy(tmpBuf, ("Site-survey request failed!")); 
					goto ss_err_end; 
					break; 
				default: 
					break; 
			} 
			// ==== GANTOE ====
/*
			if ( getWlSiteSurveyRequest(WLAN_IF,  &status) < 0 ) {
				strcpy(tmpBuf, ("Site-survey request failed!"));
				goto ss_err;
			}
*/
			if (status != 0) {	// not ready
				if (wait_time++ > 10) {
					strcpy(tmpBuf, ("scan request timeout!"));
					goto ss_err_end;
				}
#ifdef	CONFIG_RTK_MESH
		// ==== modified by GANTOE for site survey 2008/12/26 ==== 
				sleep(1 + rand()%2);//usleep(1000000 + (rand() % 2000000));
#else
				sleep(1);
#endif
			}
			else
				break;
		}

		// wait until scan completely
		wait_time = 0;
		while (1) {
			res = 1;	// only request request status
			if ( getWlSiteSurveyResult(WLAN_IF, (SS_STATUS_Tp)&res) < 0 ) {
				strcpy(tmpBuf, ("Read site-survey status failed!"));
				free(pStatus);
				pStatus = NULL;
				goto ss_err_end;
			}
			if (res == 0xff) {   // in progress
				/*prolong wait time due to scan both 2.4G and 5G */
				if (wait_time++ > 25) 		
				{
					strcpy(tmpBuf, ("scan timeout!"));
					free(pStatus);
					pStatus = NULL;
					goto ss_err_end;
				}
				sleep(1);
			}
			else
				break;
		}

		if (submitUrl[0])
			send_redirect_perm(submitUrl);

		return;
	}

	connect = get_cstream_var(postData,length, ("connect"), "");
	if ( connect[0] ) 
	{
#if defined(BRIDGE_REPEATER) && defined(DHCP_AUTO_SUPPORT) && defined(CONFIG_RTL_ULINKER)
		siteSurveyStatus = SITE_SURVEY_STATUS_RUNNING;
#endif
		char *strSSID;
		int rptEnabled=0;
		int wlan_mode=0;
		int wlanvxd_mode=0;
		char *strChannel;
		int channelIdx;
		int opmode=0;
		
		
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		apmib_get(MIB_OP_MODE, (void *)&opmode);
		if(apmib_get_wlanidx() == 0)
			apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		else
			apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);
		wlanifp = get_cstream_var(postData,length,("wlanif"), "");
			
		if( (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE) && (rptEnabled == 1)	)
			vxd_wisp_wan = 1;
		if((wlan_mode == CLIENT_MODE) && (opmode == 2))//wisp-client
			client_wisp_wan = 1;
		if(vxd_wisp_wan){
			sprintf(tmpBuf, "%s-vxd0", wlanifp);
			RunSystemCmd(NULL_FILE, "ifconfig", tmpBuf, "down", NULL_STR);
		}
		RunSystemCmd(NULL_FILE, "ifconfig", wlanifp, "down", NULL_STR);

#if defined(WLAN_PROFILE)
	/* disable wireless profile first */
	if(apmib_get_wlanidx()== 0)
	{
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}
	profileEnabledVal = 0;
	
	apmib_get(profile_enabled_id, (void *)&oriProfileEnabledVal);

//printf("\r\n oriProfileEnabledVal=[%d],__[%s-%u]\r\n",oriProfileEnabledVal,__FILE__,__LINE__);

	apmib_set(profile_enabled_id, (void *)&profileEnabledVal);

#endif //#if defined(WLAN_PROFILE)

		
#if defined(CONFIG_RTL_92D_SUPPORT)	
		int phyBand;
		int i;
		unsigned char wlanIfStr[10];
		int band2g5gselect=0;
		apmib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&band2g5gselect);
		if(band2g5gselect != BANDMODEBOTH)
		{
#ifdef CONFIG_WLANIDX_MUTEX
			int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif			
			for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
			{
				unsigned char wlanif[10];
				memset(wlanif,0x00,sizeof(wlanif));
				sprintf(wlanif, "wlan%d",i);
				if(SetWlan_idx(wlanif))
				{
					int intVal = 1;
					apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);
					
				}						
			}
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif
			strChannel = get_cstream_var(postData,length, ("pocket_channel"), "");
			
			if(strChannel[0])
			{
				short wlanif;
				
				channelIdx = atoi(strChannel);
				
				if(channelIdx > 14) // connect to 5g AP
					phyBand = PHYBAND_5G;
				else
					phyBand = PHYBAND_2G;
					
				wlanif = whichWlanIfIs(phyBand);
				
				memset(wlanIfStr,0x00,sizeof(wlanIfStr));		
				sprintf(wlanIfStr, "wlan%d",wlanif);

#ifdef CONFIG_WLANIDX_MUTEX
				int s2 = apmib_save_idx();
#else
				apmib_save_idx();
#endif				
				if(SetWlan_idx(wlanIfStr))
				{
					int val;
					val = 0;
					apmib_set(MIB_WLAN_WLAN_DISABLED, (void *)&val);												
					
					apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
																
#if defined(CONFIG_RTL_ULINKER)						
					if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
						;
					else
#endif
						wlan_mode = CLIENT_MODE;
						
					apmib_set(MIB_WLAN_MODE, (void *)&wlan_mode);												
				}
#ifdef CONFIG_WLANIDX_MUTEX
					apmib_revert_idx(s2);
#else
					apmib_revert_idx();
#endif				
				/* we can't up wlan1 alone, so we swap wlan0 and wlan1 settings */
				if(wlanif != 0)
				{
					swapWlanMibSetting(0,wlanif);			
				}					
			}
		
		}
		
		
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)
		
		
//printf("\r\n ++++++++++++wlanifp=[%s],__[%s-%u]\r\n",wlanifp,__FILE__,__LINE__);
#if defined(CONFIG_RTL_ULINKER)
		//in ulinker project, we save settings to root interface, than clone to rpt interface
#else
		if(vxd_wisp_wan== 1)
		{
			sprintf(wlanifp_bak, "%s", wlanifp);
			sprintf(wlanifp, "%s-vxd0", wlanifp_bak);
		}		
#endif
		
		SetWlan_idx(wlanifp);
		
		strSSID = get_cstream_var(postData,length, ("pocketAP_ssid"), "");
    //strSSID is BSSID, below transfer it to SSID. This fix the issue of AP ssid contains "		
    		for (k=0; k<pStatus->number && pStatus->number!=0xff; k++) 
    		{
    			pBss = &pStatus->bssdb[k];
    			snprintf(tmpBuf, 200, ("%02x:%02x:%02x:%02x:%02x:%02x"),
    			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
    			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);

    			if(strcmp(strSSID, tmpBuf)==0)
    			{
    				memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
    				ssidbuf[pBss->bdSsId.Length] = '\0';	
    				break;
    			}		
    		}
    		apmib_set(MIB_WLAN_SSID, (void *)ssidbuf);

#if 1//def CONFIG_SMART_REPEATER
		if(vxd_wisp_wan== 1)
			if(apmib_get_wlanidx() == 0)
				apmib_set(MIB_REPEATER_SSID1, (void *)ssidbuf);
			else
				apmib_set(MIB_REPEATER_SSID2, (void *)ssidbuf);

#endif
		apmib_update(CURRENT_SETTING);

#if defined(CONFIG_RTL_ULINKER)								
		strChannel = get_cstream_var(postData,length, ("pocket_channel"), "");
			
		if(strChannel[0])
		{
			channelIdx = atoi(strChannel);
			if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
			{
				apmib_set( MIB_WLAN_CHANNEL,  (void *)&channelIdx);
			}
		}						
#endif		
						
		strSel = get_cstream_var(postData,length, ("select"), "");
		if (strSel[0]) {
			unsigned char res;
			NETWORK_TYPE_T net;
			int chan;

			if (pStatus == NULL) {
				strcpy(tmpBuf, ("Please refresh again!"));
				goto ss_err;

			}
			sscanf(strSel, "sel%d", &idx);
			if ( idx >= pStatus->number ) { // invalid index
				strcpy(tmpBuf, ("Connect failed 1!"));
				goto ss_err;
			}
#ifdef CONFIG_RTK_MESH 
// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
			if(mesh_enable) 
			{ 
				int i, mesh_index, tmp_index; 
				char original_mesh_id[MESHID_LEN];
				int original_channel = 0;
				
				// backup related info. 
				strcpy(original_mesh_id, "channel");
				if(!apmib_get( MIB_WLAN_CHANNEL,  (void *)&original_channel))
				{
					strcpy(tmpBuf, ("get MIB_CHANNEL error!"));
					goto ss_err;
				}
				else
				{
					original_channel = *(int*)original_mesh_id;
				}
				if(apmib_get(MIB_WLAN_MESH_ID, (void*)original_mesh_id) == 0)
				{
					strcpy(tmpBuf, ("get MIB_WLAN_MESH_ID error!"));
					goto ss_err;
				}
				
				// send connect request to the driver
				for(tmp_index = 0, mesh_index = 0; tmp_index < pStatus->number && pStatus->number != 0xff; tmp_index++) 
				if(pStatus->bssdb[idx].bdMeshId.Length > 0 && mesh_index++ == idx) 
					break; 
				idx = tmp_index;
				pMsg = "Connect failed 2!!";
				if(!setWlJoinMesh(WLAN_IF, pStatus->bssdb[idx].bdMeshIdBuf - 2, pStatus->bssdb[idx].bdMeshId.Length, pStatus->bssdb[idx].ChannelNumber, 0)) // the problem of padding still exists 
				{ 
					// check whether the link has established
					for(i = 0; i < 10; i++)	// This block might be removed when the mesh peerlink precedure has been completed
					{
						if(!getWlMeshLink(WLAN_IF, pStatus->bssdb[idx].bdBssId, 6))
						{
							char tmp[MESHID_LEN]; 
							int channel; 
							memcpy(tmp, pStatus->bssdb[idx].bdMeshIdBuf - 2, pStatus->bssdb[idx].bdMeshId.Length); // the problem of padding still exists 
							tmp[pStatus->bssdb[idx].bdMeshId.Length] = '\0'; 
							if ( apmib_set(MIB_WLAN_MESH_ID, (void *)tmp) == 0)
							{ 
								strcpy(tmpBuf, ("Set MeshID error!")); 
								goto ss_err; 
							} 
							// channel = pStatus->bssdb[idx].ChannelNumber; 
							channel = 0; // requirement of Jason, not me 
							if ( apmib_set(MIB_WLAN_CHANNEL, (void*)&channel) == 0)
							{ 
								strcpy(tmpBuf, ("Set Channel error!")); 
								goto ss_err; 
							} 
							apmib_update_web(CURRENT_SETTING); 
							pMsg = "Connect successfully!!"; 
							break;
						}
						sleep(3);//usleep(3000000);
					}
				}
				// if failed, reset to the original channel
				if(strcmp(pMsg, "Connect successfully!!"))
				{
					setWlJoinMesh(WLAN_IF, original_mesh_id, strlen(original_mesh_id), original_channel, 1);
				}
			} 
			else 
// ==== GANTOE ==== 
#endif 
			{ 
#if 1
                                unsigned char wlan_idx=0;
                                char *tmpStr=NULL, *wlanif=NULL;
                                char wlan_vxd_if[20]={0};
                                char varName[20]={0};
                                unsigned int i=0,val=0;
                                wlanif = get_cstream_var(postData,length, ("wlanif"), "");
                                //SetWlan_idx(tmpStr);
 
                                tmpStr = get_cstream_var(postData,length, ("wlan_idx"), "");
                                if(tmpStr[0])
                                        wlan_idx = atoi(tmpStr);
 
                                sprintf(varName, "method%d", wlan_idx);
 
                                tmpStr = get_cstream_var(postData,length,varName, "");
                                if(tmpStr[0])
                                {
                                        val = atoi(tmpStr);
                                        if(val == ENCRYPT_DISABLED)
                                        {
                                                ENCRYPT_T encrypt = ENCRYPT_DISABLED;
                                                apmib_set( MIB_WLAN_ENCRYPT, (void *)&encrypt);
                                        }
                                        else if(val == ENCRYPT_WEP)
                                        {
                                                if(wepHandler(postData,length,tmpBuf, wlan_idx) < 0)
                                                {
                                                        goto ss_err;
                                                }
                                        }
                                        else if(val > ENCRYPT_WEP && val <= ENCRYPT_WPA2_MIXED)
                                        {
                                                if(wpaHandler(postData,length, tmpBuf, wlan_idx) < 0)
                                                {
                                                        goto ss_err;
                                                }
                                        }
#ifdef CONFIG_RTL_WAPI_SUPPORT
						 else if(val == ENCRYPT_WAPI){
						 	if(wpaHandler(postData,length, tmpBuf, wlan_idx) < 0)
                                             {
                                                        goto ss_err;
                                             }
						 }
#endif
					}
#else                                
			// check encryption type match or not
			if ( !apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt) ) {
				strcpy(tmpBuf, ("Check encryption error!"));
				goto ss_err;
			}
			else {
				// no encryption
				if (encrypt == ENCRYPT_DISABLED)
				{
					if (pStatus->bssdb[idx].bdCap & 0x00000010) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
				// legacy encryption
				else if (encrypt == ENCRYPT_WEP)
				{
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] != 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
#if defined(CONFIG_RTL_WAPI_SUPPORT)
				else if (encrypt == ENCRYPT_WAPI)
				{
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] != SECURITY_INFO_WAPI) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else
						; // success
				}
#endif
				// WPA/WPA2
				else
				{
					int isPSK, auth;
					apmib_get(MIB_WLAN_WPA_AUTH, (void *)&auth);
					if (auth == WPA_AUTH_PSK)
						isPSK = 1;
					else
						isPSK = 0;					
								
#ifdef CONFIG_RTL_802_1X_CLIENT_SUPPORT
					wpaPSK=isPSK;
#endif
								
					if ((pStatus->bssdb[idx].bdCap & 0x00000010) == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (pStatus->bssdb[idx].bdTstamp[0] == 0) {
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					else if (encrypt == ENCRYPT_WPA) {
						if (((pStatus->bssdb[idx].bdTstamp[0] & 0x0000ffff) == 0) || 
								(isPSK && !(pStatus->bssdb[idx].bdTstamp[0] & 0x4000)) ||
								(!isPSK && (pStatus->bssdb[idx].bdTstamp[0] & 0x4000)) ) {						
						strcpy(tmpBuf, ("Encryption type mismatch!"));
						goto ss_err;
					}
					}
					else if (encrypt == ENCRYPT_WPA2) {
						if (((pStatus->bssdb[idx].bdTstamp[0] & 0xffff0000) == 0) ||
								(isPSK && !(pStatus->bssdb[idx].bdTstamp[0] & 0x40000000)) ||
								(!isPSK && (pStatus->bssdb[idx].bdTstamp[0] & 0x40000000)) ) {
							strcpy(tmpBuf, ("Encryption type mismatch!"));
							goto ss_err;
						}
					}	
					else
						; // success
				}
			}
#endif

#if 0
			// Set SSID, network type to MIB
			memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
			tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
			
			memset(tmpBuf,0x00,sizeof(tmpBuf));
			
			tmpStr = req_get_cstream_var(wp, ("pocketAP_ssid"), "");
			if(tmpStr[0])
				sprintf(tmpBuf,"%s",tmpStr);
			
//printf("\r\n tmpBuf=[%s],__[%s-%u]\r\n",tmpBuf,__FILE__,__LINE__);
			
			if ( apmib_set(MIB_WLAN_SSID, (void *)tmpBuf) == 0) {
				strcpy(tmpBuf, ("Set SSID error!"));
				goto ss_err;
			}
#endif

			if ( pStatus->bssdb[idx].bdCap & cESS )
				net = INFRASTRUCTURE;
			else
				net = ADHOC;
			
			if ( apmib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
				strcpy(tmpBuf, ("Set MIB_WLAN_NETWORK_TYPE failed!"));
				goto ss_err;
			}

			//add temporary for Ecos repeater connect to different channel fail issue.
			if(vxd_wisp_wan)
			{
				SetWlan_idx(wlanifp_bak);				
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0)
   					strcpy(tmpBuf, ("Set channel number error!"));
				int sideband;
				if (pStatus->bssdb[idx].bdTstamp[1] & 0x00000004)
                                	sideband = 0;/*HT_2NDCH_OFFSET_BELOW*/
                                else
                                	sideband = 1;/*HT_2NDCH_OFFSET_ABOVE*/
                                apmib_set( MIB_WLAN_CONTROL_SIDEBAND, (void *)&sideband);
				if ((chan<=4 && sideband==0) || (chan>=10 && sideband==1)){
					sideband = ((sideband + 1) & 0x1);
					apmib_set( MIB_WLAN_CONTROL_SIDEBAND, (void *)&sideband);
				}
				SetWlan_idx(wlanifp);				
			}
			
			if (net == ADHOC) {
				chan = pStatus->bssdb[idx].ChannelNumber;
				if ( apmib_set( MIB_WLAN_CHANNEL, (void *)&chan) == 0) {
   					strcpy(tmpBuf, ("Set channel number error!"));
					goto ss_err;
				}
				int is_40m_bw ;// = (pStatus->bssdb[idx].bdTstamp[1] & 2) ? 1 : 0;
				if(pStatus->bssdb[idx].bdTstamp[1] & 64)
					is_40m_bw = 2;
				else if(pStatus->bssdb[idx].bdTstamp[1] & 2)
					is_40m_bw = 1;
				else
					is_40m_bw = 0;
				apmib_set(MIB_WLAN_CHANNEL_BONDING, (void *)&is_40m_bw);				
			}

#if defined(CONFIG_RTL_ULINKER) //repeater mode: clone wlan setting to wlan-vxd and modify wlan ssid

		if(wlan_mode != CLIENT_MODE && wlan_mode != WDS_MODE && rptEnabled == 1)
		{
			int isUpnpEnabled=0;
#ifdef CONFIG_WLANIDX_MUTEX
			int s = apmib_save_idx();
#else
			apmib_save_idx();
#endif			
			//int ori_vwlan_idx = vwlan_idx;
			char ssidBuf[64];
			

			apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
			//vwlan_idx = NUM_VWLAN_INTERFACE;
			
			
			/* get original setting in vxd interface */
			apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&isUpnpEnabled);
			apmib_get(MIB_WLAN_MODE, (void *)&wlanvxd_mode);
			
									
			ulinker_wlan_mib_copy(&pMib->wlan[wlan_idx][NUM_VWLAN_INTERFACE], &pMib->wlan[wlan_idx][0]);
			
			/* restore original setting in vxd interface and repeater ssid*/			
			apmib_set(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&isUpnpEnabled);
			apmib_set(MIB_WLAN_MODE, (void *)&wlanvxd_mode);

#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif		
			//vwlan_idx = ori_vwlan_idx;
			
			/* add "-ext" at last of wlan ssid */
			apmib_get( MIB_WLAN_SSID,  (void *)ssidBuf);

			if(wlan_idx == 0)
				apmib_set(MIB_REPEATER_SSID1, (void *)&ssidBuf);
			else
				apmib_set(MIB_REPEATER_SSID2, (void *)&ssidBuf);

			
			if(strlen(ssidBuf)<sizeof(ssidBuf)+4)
			{
#if defined(HAVE_TWINKLE_RSSI) 
#else
				strcat(ssidBuf,"-ext");
#endif
				apmib_set( MIB_WLAN_SSID,  (void *)ssidBuf);
				apmib_set( MIB_WLAN_WSC_SSID, (void *)ssidBuf);
			}
		}
#endif


			apmib_update_web(CURRENT_SETTING);

#if 1 //reinit wlan interface and mib
			if(vxd_wisp_wan)
				SetWlan_idx(wlanifp_bak);

			RunSystemCmd(NULL_FILE, "flash", "set_mib", wlanif, NULL_STR);
  			RunSystemCmd(NULL_FILE, "ifconfig", wlanif, "up", NULL_STR);
#if 1//def CONFIG_SMART_REPEATER
			if(vxd_wisp_wan){
				sprintf(tmpBuf, "%s-vxd0", wlanif);
				RunSystemCmd(NULL_FILE, "flash", "set_mib", tmpBuf, NULL_STR);
				RunSystemCmd(NULL_FILE, "ifconfig", tmpBuf, "up", NULL_STR);
				SetWlan_idx(wlanifp);
                    }
#endif                        
					
		//#if defined(CONFIG_RTL_ULINKER)
		//	run_init_script_flag = 1;
		//#else
			 // wlan0 entering forwarding state need some time
			sleep(3);

			_Start_Wlan_Applications();
		//#endif
#endif

			res = idx;
			wait_time = 0;

#if 0	//Because reinit wlan app above, so don't need wlJoinRequest here.
			while (1) {
				if ( getWlJoinRequest(WLAN_IF, &pStatus->bssdb[idx], &res) < 0 ) {
					strcpy(tmpBuf, ("Join request failed!"));
					goto ss_err;
				}
				if ( res == 1 ) { // wait
				#if defined (CONFIG_RTL_92D_SUPPORT)  && defined (CONFIG_POCKET_ROUTER_SUPPORT)
					/*prolong join wait time for pocket ap*/
					if (wait_time++ > 10) 
				#else
					if (wait_time++ > 5) 
				#endif
					{
						strcpy(tmpBuf, ("connect-request timeout!"));
						goto ss_err;
					}
					sleep(1);
					continue;
				}
				break;
			}

			if ( res == 2 ) // invalid index
				pMsg = "Connect failed 3!";
			else 
			{
				wait_time = 0;
				while (1) {
					if ( getWlJoinResult(WLAN_IF, &res) < 0 ) {
						strcpy(tmpBuf, ("Get Join result failed!"));
						goto ss_err;
					}
					if ( res != 0xff ) { // completed
					

						break;
					}
					else
					{
						if (wait_time++ > 10) {
							strcpy(tmpBuf, ("connect timeout!"));
							goto ss_err;
						}
					}
					sleep(1);
				}

				if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
					pMsg = "Connect failed 4!";
				else {					
					status = 0;
					
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
					
					//if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2) {
					if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2 || encrypt == ENCRYPT_WAPI) {
						bss_info bss;
						wait_time = 0;
						
						max_wait_time=10;	//Need more test, especially for 802.1x client mode
						
						while (wait_time++ < max_wait_time) {
							getWlBssInfo(WLAN_IF, &bss);
							if (bss.state == STATE_CONNECTED){
								break;
							}
							sleep(1);
						}
						if (wait_time > max_wait_time)						
							status = 1;
					}

					if (status)
						pMsg = "Connect failed 5!";
					else
						pMsg = "Connect successfully!";
				}
			}
#else
			{
				wait_time = 0;
#if 1//def CONFIG_SMART_REPEATER
				if(vxd_wisp_wan && !strstr(WLAN_IF,"-vxd"))
					strcat(WLAN_IF,"-vxd0");
				
#if defined(CONFIG_RTL_DFS_SUPPORT)
				if(strstr(WLAN_IF,"-vxd"))
				{
					connect_timeout = 70;
				}
#endif
				
#endif
				while (1) 
				{
					if ( getWlJoinResult(WLAN_IF, &res) < 0 ) {
						strcpy(tmpBuf, ("Get Join result failed!"));
						goto ss_err;
					}

					if(res==STATE_Bss  || res==STATE_Ibss_Idle || res==STATE_Ibss_Active) { // completed 
						break;
					}
					else
					{

						if (wait_time++ > connect_timeout) 
						{
							strcpy(tmpBuf, ("connect timeout!"));
							goto ss_err;
						}
					}
					sleep(1);
				}

				//if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
				//	pMsg = (unsigned char *)"Connect failed 4!";
				//else {					
					status = 0;
					
					apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);
					
					//if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2) {
					if(net == ADHOC)
						status = 0;
					else if (encrypt == ENCRYPT_WPA || encrypt == ENCRYPT_WPA2 || encrypt == ENCRYPT_WAPI) {
						bss_info bss;
						wait_time = 0;
						
						max_wait_time=25;	//Need more test, especially for 802.1x client mode
						
						while (wait_time++ < max_wait_time) {
							getWlBssInfo(WLAN_IF, &bss);
							if (bss.state == STATE_CONNECTED){
								break;
							}
							sleep(1);
						}
						if (wait_time > max_wait_time)						
							status = 1; //fail
					}

					if (status)
						pMsg =  (unsigned char *)"Connect failed 5!";
					else
						pMsg =  (unsigned char *)"Connect successfully!";

				//}
			}
#endif
		}


#if defined(WLAN_PROFILE)
		apmib_set(profile_enabled_id, (void *)&oriProfileEnabledVal);
		apmib_update_web(CURRENT_SETTING);
#endif //#if defined(WLAN_PROFILE)


#if defined(CONFIG_POCKET_AP_SUPPORT)  
			if(!status)
			{
				pMsg = "Connect successfully! Please wait while rebooting.";
				OK_MSG1(pMsg, submitUrl);
				sleep(2);
				system("reboot");
			} 
#elif defined(CONFIG_POCKET_ROUTER_SUPPORT)// || defined(CONFIG_RTL_ULINKER)
			{
#ifndef NO_ACTION
				run_init_script("all");
#endif
				REBOOT_WAIT("/wizard.htm");
				//REBOOT_WAIT("/wlsurvey.htm");
				
			} 
#else
			{
				//FIX ME .HF
				//OK_MSG1(pMsg,submitUrl);
				if(vxd_wisp_wan || client_wisp_wan)
				{
					unsigned int wan_type=0;
					unsigned int lan_type=0;
					printf("%s:%d\n",__FUNCTION__,__LINE__);
					apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
					apmib_get(MIB_DHCP, (void *)&lan_type);
					if(lan_type==DHCP_LAN_CLIENT
#ifdef DHCP_AUTO_SUPPORT
					|| 1
#endif
					)
					{
#ifdef HAVE_DHCPC
						dhcpc_reconnect(1);
#endif
					}
					if(wan_type == STATIC_IP)
					{
						//don't need to reset wan related daemon
						//set_staticIP();
						//kick_event(WAN_EVENT);
					}
#if defined(HAVE_DHCPC)
					else if(wan_type == DHCP_CLIENT)
					{
						dhcpc_reconnect(1);
					}
#endif
#if defined(HAVE_PPPOE)
					else if(wan_type == PPPOE)
					{
						pppoe_reconnect();
					}
#endif
				}
#if defined(WLAN_PROFILE)
				ret_survey_page(pMsg,submitUrl, (status?0:1), apmib_get_wlanidx(), isWizard);
#else
#if defined(BRIDGE_REPEATER) && defined(DHCP_AUTO_SUPPORT) && defined(CONFIG_RTL_ULINKER)
				CONNECT_OK_MSG(pMsg);
#else
				OK_MSG1(pMsg,submitUrl);	
#endif
#endif
				//RET_SURVEY_PAGE(pMsg, submitUrl, (status?0:1), apmib_get_wlanidx(0, isWizard);
			}
#endif
		}
#if 1//defined(CONFIG_SMART_REPEATER)
		if(vxd_wisp_wan)
		{
			char*ptmp;
			SetWlan_idx(wlanifp_bak);
			ptmp=strstr(WLAN_IF,"-vxd");
			if(ptmp)
				memset(ptmp,0,sizeof(char)*strlen("-vxd0"));
		}
#endif



	}
	return;

ss_err:
#if defined(BRIDGE_REPEATER) && defined(DHCP_AUTO_SUPPORT) && defined(CONFIG_RTL_ULINKER)
       siteSurveyStatus = SITE_SURVEY_STATUS_OFF;
#endif
	if(vxd_wisp_wan)
	{
		SetWlan_idx(wlanifp_bak);
		if(!is_interface_up(wlanifp_bak))
			interface_up(wlanifp_bak);
	}
	if(!is_interface_up(wlanifp))
			interface_up(wlanifp);
ss_err_end:
#if defined(WLAN_PROFILE)
						apmib_set(profile_enabled_id, (void *)&oriProfileEnabledVal);
						apmib_update_web(CURRENT_SETTING);
#endif //#if defined(WLAN_PROFILE)				

	ERR_MSG(tmpBuf);
	
}
#if defined(WLAN_PROFILE)
#ifndef PROFILE_BUFF_SIZE
#define PROFILE_BUFF_SIZE 512
#endif
int wlProfileTblList(int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WLAN_PROFILE_T entry;
	
	char *profileBuf=(char*)malloc(PROFILE_BUFF_SIZE);
	int wlan_idx=apmib_get_wlanidx();
	int profile_num_id, profile_tbl_id;
	if(!profileBuf)
	{
		fprintf(stderr,"malloc fail!!\n");
		return -1;
	}	

	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);

	nBytesSent = snprintf(profileBuf, TMP_BUF_SIZE, ("<tr>"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>SSID</b></font></td>\n"
      	"<td align=center width=\"\" bgcolor=\"#808080\"><font size=\"2\"><b>Encrypt</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
	cyg_httpd_write_chunked(profileBuf,nBytesSent);

	for (i=1; i<=entryNum; i++) {
		unsigned char wlSecurityBuf[43]={0};
		
		*((char *)&entry) = (char)i;
		if ( !apmib_get(profile_tbl_id, (void *)&entry))
			goto PROFILE_TBL_LIST;

		if(entry.encryption == WEP64 || entry.encryption == WEP128)
		{
			sprintf(wlSecurityBuf, "%s", "WEP");
		}
		else if(entry.encryption == 3 ) //WPA
		{
			sprintf(wlSecurityBuf, "%s", "WPA/");
			if(entry.wpa_cipher == 2)
				strcat(wlSecurityBuf, "TKIP");
			else
				strcat(wlSecurityBuf, "AES");
		}
		else if(entry.encryption == 4 ) //WPA2
		{
			sprintf(wlSecurityBuf, "%s", "WPA2/");
			if(entry.wpa_cipher == 2)
				strcat(wlSecurityBuf, "TKIP");
			else
				strcat(wlSecurityBuf, "AES");
		}
		else
			sprintf(wlSecurityBuf, "%s", "OPEN");

		
		nBytesSent = snprintf(profileBuf, TMP_BUF_SIZE, ("<tr>"
      			"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
       			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				entry.ssid,wlSecurityBuf, i);
		cyg_httpd_write_chunked(profileBuf,nBytesSent);
	}
PROFILE_TBL_LIST:
	if(profileBuf) free(profileBuf);
	return 0;
}
#ifndef PROFILE_LIST_BUFF_SIZE
#define PROFILE_LIST_BUFF_SIZE 128
#endif
int wlProfileList(int argc, char **argv)
{
	int nBytesSent=0, entryNum, i;
	WLAN_PROFILE_T entry;
	int profile_num_id, profile_tbl_id, profile_enabled_id, wlProfileEnabled;
	int wlan_idx=apmib_get_wlanidx();
	char tmpBuf[PROFILE_LIST_BUFF_SIZE]={0};
	
	if(wlan_idx == 0)
	{
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
		profile_enabled_id = MIB_PROFILE_ENABLED1;
	}
	else
	{
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
		profile_enabled_id = MIB_PROFILE_ENABLED2;
	}

	apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);
	if(wlProfileEnabled == 0)
	{
		nBytesSent = snprintf(tmpBuf, PROFILE_LIST_BUFF_SIZE,"%s","//wireless profile disabled");
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		return 0;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);


	for (i=1; i<=entryNum; i++) {		
		unsigned char encryptBuf[43]={0};
		unsigned char wepBuf[43]={0};		
		int wpaCipherVal = 0;
		
		sprintf(wepBuf, "%d", 0); 
		//Ssid|Encrypt|Authtype|wep|Wepkeytype|wpaCipher|wpa2Cipher|Pskformat

		memset(&entry, 0x00, sizeof(WLAN_PROFILE_T));
		*((char *)&entry) = (char)i;
		if ( !apmib_get(profile_tbl_id, (void *)&entry))
			return -1;

		if(entry.encryption == WEP64 || entry.encryption == WEP128)
		{
			sprintf(encryptBuf, "%d", 1);
			if(entry.encryption == WEP64)
				sprintf(wepBuf, "%d", 1);
			else
				sprintf(wepBuf, "%d", 2); //128
		}
		else if(entry.encryption == 3 ) //WPA
		{
			sprintf(encryptBuf, "%d", 2);
		}
		else if(entry.encryption == 4 ) //WPA2
		{
			sprintf(encryptBuf, "%d", 4);
		}
		else
		{
			sprintf(encryptBuf, "%d", 0);
		}

		if(entry.wpa_cipher == 2)
			wpaCipherVal = 1;
		else
			wpaCipherVal = 2;
		
		//Ssid|Encrypt|Authtype|wep|Wepkeytype|wpaCipher|wpa2Cipher|Pskformat|Wpapsk
		nBytesSent = snprintf(tmpBuf, PROFILE_LIST_BUFF_SIZE, ("token[%d] =\'%s|%s|%d|%s|%d|%d|%d|%d|%s\';\n"),(i-1), entry.ssid,encryptBuf, entry.auth, wepBuf, entry.wepKeyType, wpaCipherVal, wpaCipherVal, entry.wpaPSKFormat, entry.wpaPSK );
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);	
	}
	return 0;

}
#ifndef PROFILE_INFO_BUFFER_SIZE
#define PROFILE_INFO_BUFFER_SIZE 64
#endif
void getWlProfileInfo(int argc, char **argv)
{
	char	*name;
	int idx;
	int profile_enabled_id, profile_num_id, profile_tbl_id;
	int entryNum;
	WLAN_PROFILE_T entry;
	int wlProfileEnabled;
	int wlan_idx=apmib_get_wlanidx();
	char tmpBuf[PROFILE_INFO_BUFFER_SIZE]={0};
	int nBytesSent=0;
	int buffer[64];
	name = argv[0];
	idx = atoi(argv[1]);
	
	if (name == NULL) {
   		fprintf(stderr, "Insufficient args\n");
   		return -1;
   	}

	if(wlan_idx == 0)
	{
		profile_enabled_id = MIB_PROFILE_ENABLED1;
		profile_num_id = MIB_PROFILE_NUM1;
		profile_tbl_id = MIB_PROFILE_TBL1;
	}
	else
	{
		profile_enabled_id = MIB_PROFILE_ENABLED2;
		profile_num_id = MIB_PROFILE_NUM2;
		profile_tbl_id = MIB_PROFILE_TBL2;
	}

	apmib_get( profile_enabled_id, (void *)&wlProfileEnabled);
	if(wlProfileEnabled == 0)
	{
		nBytesSent = snprintf(tmpBuf, PROFILE_INFO_BUFFER_SIZE,"%s","0");
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		return 0;
	}
	
	apmib_get(profile_num_id, (void *)&entryNum);
	if(idx > entryNum)
	{
		nBytesSent = snprintf(tmpBuf, PROFILE_INFO_BUFFER_SIZE,"%s","0");
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		return 0;
	}
	
	*((char *)&entry) = (char)idx;
	if ( !apmib_get(profile_tbl_id, (void *)&entry))
	{
		nBytesSent = snprintf(tmpBuf, PROFILE_INFO_BUFFER_SIZE,"%s","0");
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		return 0;
	}
		
//////MENU//////////////////////////////////////////////////
	if(!strcmp(name,"wlProfileTblEnable"))
	{
		nBytesSent = snprintf(tmpBuf, PROFILE_INFO_BUFFER_SIZE,"%d",1);
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		
		return 0;
	}
	else if(!strcmp(name,"wlProfileSSID"))
	{
		sprintf(buffer,"%s",entry.ssid);
		translate_control_code(buffer);
		nBytesSent = snprintf(tmpBuf, PROFILE_INFO_BUFFER_SIZE,"%s",buffer);
		cyg_httpd_write_chunked(tmpBuf,nBytesSent);
		return 0;
	}		
	return 0;
}
#endif //#if defined(WLAN_PROFILE)

