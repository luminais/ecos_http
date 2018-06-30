/*
 *	Common functions to do common works, all AP modules may call them
 *	Author: sen_liu
 *	2012/06/20
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define BRIDGE_INTERFACE_MAX_LEN 160
#define MAC_ADDR_LEN    6	//added by lynn_xu
//extern int vlanIDIsWan(unsigned short vid);

#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETWANLINKSPEED 2100
#define RTL8651_IOCTL_SETWANLINKSPEED 2101

#define RTL8651_IOCTL_GETLANLINKSTATUS 2102
#define RTL8651_IOCTL_GETLANLINKSPEED    2103
#define RTL8651_IOCTL_GETETHERLINKDUPLEX 2104

#define RTL8651_IOCTL_GET_ETHER_EEE_STATE 2105
#define RTL8651_IOCTL_GET_ETHER_BYTES_COUNT 2106
#define RTL8651_IOCTL_SET_IPV6_WAN_PORT 2107
#define RTL8651_IOCTL_SETLANLINKSPEED   2108
#define RTL8651_IOCTL_SETETHERLINKDUPLEX 2109

#define IPFW_INDEX_STEP 2
#define IPFW_SECTOR_SIZE 2000
#define NEXT_IPFW_INDEX(x)  ((x) += IPFW_INDEX_STEP)
/*0 ~ 2000 is reserved for allow or deny rule for wan input*/
#define IPFW_INDEX_START 50

#define IPFW_INDEX_NAT_DOWNLINK_BASE  10000
#define IPFW_INDEX_NAT_DOWNLINK   IPFW_INDEX_NAT_DOWNLINK_BASE
#ifdef HAVE_NAPT_NO_DYN
#define IPFW_INDEX_NAT_DOWNLINK_CHECK  (IPFW_INDEX_NAT_DOWNLINK+IPFW_SECTOR_SIZE)
//nat loopback dnat rules 
#if defined(CONFIG_RTL_NAT_LOOPBACK)
#define IPFW_INDEX_NATLOOPBACK_DNAT (IPFW_INDEX_NAT_DOWNLINK_CHECK+IPFW_SECTOR_SIZE)
#define IPFW_INDEX_FILTER (IPFW_INDEX_NATLOOPBACK_DNAT+IPFW_SECTOR_SIZE)
#else
#define IPFW_INDEX_FILTER (IPFW_INDEX_NAT_DOWNLINK_CHECK+IPFW_SECTOR_SIZE)
#endif
#else
//nat loopback dnat rules
#if defined(CONFIG_RTL_NAT_LOOPBACK)
#define IPFW_INDEX_NATLOOPBACK_DNAT  (IPFW_INDEX_NAT_DOWNLINK+IPFW_SECTOR_SIZE)
#define IPFW_INDEX_FILTER  (IPFW_INDEX_NATLOOPBACK_DNAT+IPFW_SECTOR_SIZE)
#else
#define IPFW_INDEX_FILTER  (IPFW_INDEX_NAT_DOWNLINK+IPFW_SECTOR_SIZE)
#endif
#endif
#define IPFW_INDEX_MINIIGD (IPFW_INDEX_FILTER + IPFW_SECTOR_SIZE)
#define IPFW_INDEX_QOS      (IPFW_INDEX_MINIIGD+IPFW_SECTOR_SIZE*2)
#define IPFW_INDEX_ALG       (IPFW_INDEX_QOS+IPFW_SECTOR_SIZE)
#define IPFW_INDEX_SKIP_TO_NAT_UPLINK_BASE  40000
/*rule for packet to DUT or fragment packet or icmp error packet*/
#define IPFW_INDEX_STATE_NOT_MATCH (IPFW_INDEX_SKIP_TO_NAT_UPLINK_BASE+IPFW_SECTOR_SIZE)

#define IPFW_INDEX_TRIGGER_PORT			56000
#define IPFW_INDEX_PPTP_ALLOW_RECV_FROM_WAN	    57850
#define IPFW_INDEX_IPSEC_ALLOW_RECV_FROM_WAN	57900
#define IPFW_INDEX_DENY_DOWNLINK_ALL	58000
#define IPFW_INDEX_QOS_DOWNSTREAM      58100

#if defined(CONFIG_RTL_NAT_LOOPBACK)
#define IPFW_INDEX_QOS_NEW	64000
#else
#define IPFW_INDEX_QOS_NEW	60100
#endif

#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
#define IPFW_INDEX_NAT_UPLINK2    IPFW_INDEX_NAT_UPLINK-2          
#endif
#define IPFW_INDEX_NAT_UPLINK				60000
#define IPCONFLICT_UPDATE_FIREWALL
#ifdef KLD_ENABLED
#define IPFW_RANGE_SIZE 1000
#define IPFW_INDEX_URL_FILTER   (IPFW_INDEX_NAT_UPLINK)
#define IPFW_INDEX_MAC_FILTER   (IPFW_INDEX_URL_FILTER+IPFW_RANGE_SIZE)
#define IPFW_INDEX_FIREWALL_RULES (IPFW_INDEX_MAC_FILTER+IPFW_RANGE_SIZE)
#endif

#if defined(CONFIG_RTL_NAT_LOOPBACK)
//nat loopback snat start position
#define IPFW_OFFSET_SIZE 500 
#define IPFW_INDEX_NATLOOPBACK_SNAT (IPFW_INDEX_NAT_UPLINK + IPFW_OFFSET_SIZE)
#endif

#define IPFW_INDEX_ALLOW_ALL				65531
#define _PATH_TMP_LOG "/tmp/shell_tmp.log"
#define _PATH_TMP_LOG0 "/tmp/shell_tmp0.log"

#ifdef HAVE_TELNET_USERNAME_PASSWORD
	#define TELNET_USERNAME_PASSWORD_FILE "/etc/usrnmpwd.conf"  //read user name and password from file "/etc/usrnmpwd.conf"
#endif

/* type define */
struct user_net_device_stats {
	unsigned long   rx_packets;             /* total packets received       */
	unsigned long   tx_packets;             /* total packets transmitted    */
	unsigned long   rx_bytes;               /* total bytes received         */
	unsigned long   tx_bytes;               /* total bytes transmitted      */
	unsigned long   rx_errors;              /* bad packets received         */
	unsigned long   tx_errors;              /* packet transmit problems     */
	unsigned long   rx_dropped;             /* no space in linux buffers    */
	unsigned long   tx_dropped;             /* no space available in linux  */
	unsigned long   multicast;              /* multicast packets received   */
	unsigned long   collisions;

	/* detailed rx_errors: */
	unsigned long   rx_length_errors;
	unsigned long   rx_over_errors;         /* receiver ring buff overflow  */
	unsigned long   rx_crc_errors;          /* recved pkt with crc error    */
	unsigned long   rx_frame_errors;        /* recv'd frame alignment error */
	unsigned long   rx_fifo_errors;         /* recv'r fifo overrun          */
	unsigned long   rx_missed_errors;       /* receiver missed packet       */

	/* detailed tx_errors */
	unsigned long   tx_aborted_errors;
	unsigned long   tx_carrier_errors;
	unsigned long   tx_fifo_errors;
	unsigned long   tx_heartbeat_errors;
	unsigned long   tx_window_errors;

	/* for cslip etc */
	unsigned long   rx_compressed;
	unsigned long   tx_compressed;

};
typedef enum {LINK_INIT=0, LINK_NO, LINK_DOWN, LINK_WAIT, LINK_UP};

#ifndef HAVE_TR069
typedef enum { IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
#else
typedef enum { IP_ADDR, DST_IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
#endif
int getWlanNum(void);
int getInAddr( char *interface, ADDR_T type, void *pAddr );
extern int isConnectPPP(void);
extern char *get_token(char *data, char *token);
extern int get_value(char *data, char *value);
extern unsigned char convert_atob(char *data, int base);
extern int _is_hex(char c);
extern int string_to_hex(char *string, unsigned char *key, int len);
extern int is_zero_ether_addr(const unsigned char *a);
extern int getShellOutput(char* shellCmd,char *outputBuf,int outBufLen);
#ifdef HOME_GATEWAY
int getWanInfo(char *pWanIP, char *pWanMask, char *pWanDefIP, char *pWanHWAddr);
#endif
#ifdef WIFI_SIMPLE_CONFIG
void set_wps_interface(void);
#endif
#endif
