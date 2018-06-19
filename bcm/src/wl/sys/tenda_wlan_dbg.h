
//#define TENDA_WLAN_DBG

#define MACSTR      "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a)  (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

#define TD_DBG_RADIO_CFG    0x00000001
#define TD_DBG_BIG_HAMMER   0x00000002
#define TD_DBG_EVENT        0x00000004
#define TD_DBG_AUTH         0x00000008
#define TD_DBG_ASSOC        0x00000010
#define TD_DBG_DEAUTH       0x00000020
#define TD_DBG_EAP          0x00000040
#define TD_DBG_ICMP         0x00000080
#define TD_DBG_ALL          0xFFFFFFFF
#define TD_DBG_DEFAULT_LEVEL TD_DBG_RADIO_CFG|TD_DBG_BIG_HAMMER|TD_DBG_EVENT|   \
                                TD_DBG_AUTH|TD_DBG_ASSOC|TD_DBG_DEAUTH

#define TD_DBG_SYSLOG_MASK   TD_DBG_DEFAULT_LEVEL

#ifdef TENDA_WLAN_DBG
void td_wlan_syslog(int flag,const char *fmt, ...);

#define TD_DBG(_wlc,m, _fmt, arg...)  do {          \
        	if (_wlc->td_wlan_dbg & m)               \
            	printf( _fmt, ##arg);        \
			td_wlan_syslog(m,_fmt, ##arg);  \
        }while(0)

void td_wlan_dbg_init(void *td_wlc);
void td_wlan_dbg_exit(void *td_wlc);
#else
#define TD_DBG(_wl,m, _fmt, arg...)
#endif

