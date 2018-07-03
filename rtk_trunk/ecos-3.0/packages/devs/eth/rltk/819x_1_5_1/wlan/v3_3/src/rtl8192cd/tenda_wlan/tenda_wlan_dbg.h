

#define TD_DBG_RADIO_CFG        0x00000001
#define TD_DBG_BIG_HAMMER       0x00000002
#define TD_DBG_EVENT            0x00000004
#define TD_DBG_AUTH             0x00000008
#define TD_DBG_ASSOC            0x00000010
#define TD_DBG_DISASSOC         0x00000020
#define TD_DBG_DEAUTH           0x00000040
#define TD_DBG_FREE_STA         0x00000080

#define TD_DBG_MGMT_CTRL_MASK   0x00000000

#define TD_DBG_ISOLATE          0x00000100


#define TD_DBG_EAP              0x00001000
#define TD_DBG_ICMP             0x00002000
#define TD_DBG_DHCP             0x00004000
#define TD_DBG_ARP              0x00008000
#define TD_DBG_SKB_PATH_MASK    0x0000f000

#define TD_DBG_PSK_STR          0x00010000
#define TD_DBG_PSK_HEX          0x00020000

#define TD_DBG_AUTO_WMM         0x00040000

#define TD_DBG_ALL              0xFFFFFFFF
#define TD_DBG_DEFAULT_LEVEL    TD_DBG_MGMT_CTRL_MASK

#define TD_DBG_SYSLOG_MASK      TD_DBG_DEFAULT_LEVEL

#define TD_DBG_FLAG_IS_ICMP     0x1000
#define TD_DBG_FLAG_IS_DHCP     0x2000
#define TD_DBG_FLAG_IS_EAP      0x4000
#define TD_DBG_FLAG_IS_ARP      0x8000
#define TD_DBG_FLAG_UNMASK      0x0fff

#ifdef CONFIG_TENDA_WLAN_DBG
#define TD_DBG(_priv, m, _fmt, arg...)  do { \
        if (_priv->tenda_mib.debug & m)     \
            diag_printf( _fmt, ##arg);           \
        }while(0)

void td_wlan_dbg_help(void);
void td_wlan_dbg_init(void *tenda_mib);
int td_dump_8023_skb(void *_priv, void *p, char *func, void *txcfg);
#else
#define TD_DBG(_priv, m, fmt, args...) do{}while(0)
#define td_dump_8023_skb(a,b,c,d)   do{}while(0)
#endif


