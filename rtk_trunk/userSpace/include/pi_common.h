#ifndef __PI_COMMON_H__
#define __PI_COMMON_H__

#include <ecos_oslib.h>

#include <sys/param.h>

#include <net/if.h>

#define PI_BUFLEN_4        4
#define PI_BUFLEN_8        8   	     //!< buffer length 8
#define PI_BUFLEN_16       16   	 //!< buffer length 16
#define PI_BUFLEN_32       32   	 //!< buffer length 32
#define PI_BUFLEN_64       64   	 //!< buffer length 64
#define PI_BUFLEN_128      128  	 //!< buffer length 128
#define PI_BUFLEN_256      256  	 //!< buffer length 256
#define PI_BUFLEN_512      512  	 //!< buffer length 512
#define PI_BUFLEN_1024     1024 	 //!< buffer length 1024
#define PI_BUFLEN_2048     2048 	 //!< buffer length 2048

#define PI_MAC_STRING_LEN	18			/* MAC字符串长度 */
#define PI_IP_STRING_LEN	16				/* IP字符串长度 */
#ifdef __CONFIG_APCLIENT_CLIENT_INFO__
#define PI_DEVNAME_STRING_LEN	255		/* DEVNAME字符串长度 */
#endif
typedef unsigned long long 	PIU64;
typedef long long 			PI64;
typedef unsigned int		PIU32;
typedef int					PI32;
typedef short int           PI16;
typedef unsigned short      PIU16;
typedef unsigned char		PIU8;
typedef char				PI8;
typedef char *				PI8_P;

#if 0
#define CON_COLOR_END               "\033[m"
#define CON_COLOR_GREEN             "\033[1;32m"
#define CON_COLOR_RED               "\033[1;31m"
#define CON_COLOR_PURPLE            "\033[1;35m"
#define CON_COLOR_YELLOW            "\033[1;33m"
#define CON_COLOR_CYAN              "\033[1;36m"
#else
#define CON_COLOR_END               ""
#define CON_COLOR_GREEN             ""
#define CON_COLOR_RED               "[ERROR] "
#define CON_COLOR_PURPLE            "[WARNING] "
#define CON_COLOR_YELLOW            "[DEBUG] "
#define CON_COLOR_CYAN              ""
#endif

#define MAIN "MAIN"

#define ATE "ATE"

#define MSG "MSG"

#define GPI "GPI"

#define TPI "TPI"

#define API "API"

#define PI_ERROR(PI_NAME,fmt, arg...) \
        do {printf(CON_COLOR_RED "[%s->%s->%d]:" CON_COLOR_END fmt, PI_NAME , __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_WARNING(PI_NAME,fmt, arg...) \
        do {printf(CON_COLOR_PURPLE "[%s->%s->%d]:" CON_COLOR_END fmt,PI_NAME, __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_DEBUG(PI_NAME,fmt, arg...) \
        do {printf(CON_COLOR_YELLOW "[%s->%s->%d]:" CON_COLOR_END fmt,PI_NAME, __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_PRINTF(PI_NAME,fmt, arg...) \
        do { printf(CON_COLOR_GREEN "[%s->%s->%d]:" CON_COLOR_END fmt,PI_NAME, __FUNCTION__, __LINE__, ##arg); } while (0)

#ifndef RTL819X
#define RTL819X 1
#endif

#define BR_NAME "lan_br"
#define BR_NAMES "lan_ifnames"
#define BRS_NAME "lan%d_br"
#define BRS_NAMES "lan%d_ifnames"

#define BR_WIRED_NAME "eth"
#define BR_WIRLESS_24G_NAME "wlan1"
#define BR_WIRLESS_5G_NAME "wlan0"


#ifndef MAX_NO_BRIDGE
#define MAX_NO_BRIDGE 1
#endif

#ifndef IFUP
#define IFUP	(IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) > (b) ? (b) : (a))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifndef isdigit
#define isdigit(d) (((d) >= '0') && ((d) <= '9'))
#endif

#ifndef timeout
#define timeout cyg_timeout
#endif

#define strcpy__(d,s) strncpy(d,s,strlen(s)+1)

typedef enum
{
	RET_ERR = 0,
	RET_SUC = 1,
}RET_INFO;

typedef enum
{
	BR_DOWN = 0,
	BR_UP = 1,
}BR_ACTION;

typedef enum
{
	BR_WIRED = 0,
	BR_WIRLESS = 1,
}BR_TYPE;

typedef enum
{
	MODULE_BEGIN = 0,
	MODULE_COMPLETE = 1,
} MODULE_WORK_STATUS;
/*联网标志*/
typedef enum
{
	INTERNET_NO = 0,
	INTERNET_TRY,
	INTERNET_YES,
}INTERNET_INFO;

typedef enum
{
    BUTTON_DOWN             = 1,
    BUTTON_CONNECTED        = 2,
} BUTTONACTION;

typedef enum
{
    COLOR_ERR               = 1,
    COLOR_TRY               = 2,
    COLOR_SUC               = 3,
} COLORACTION;

typedef enum
{
    TIME_NONE               = 0,
    TIME_SHOW               = 1,
} TIMEACTION;

typedef enum
{
    WL_AP_MODE              = 0,
    WL_CLIENT_MODE		=1,
} WL_WORK_MODE;

typedef enum
{
    WL_ROUTE_MODE              = 0,
    WL_WISP_MODE            = 1,
    WL_APCLIENT_MODE        = 2,
    WL_WDS_MODE             = 3,
    WL_BRIDGEAP_MODE        = 4,
} SYS_WORK_MODE;

typedef enum
{
    WAN_NONE_MODE           = 0,
	WAN_STATIC_MODE 		= 1,
    WAN_DHCP_MODE           = 2,
    WAN_PPPOE_MODE          = 3,
	WAN_MAX_MODE			= 4, 
} WANMODE;

typedef enum
{
    WAN_NO_WIRE             = 0,
    WAN_DISCONNECTED        = 1,
    WAN_CONNECTING          = 2,
    WAN_CONNECTED           = 3,
} WANSTATUS;

typedef enum
{
    WIFI_OK                  = 0,
	WIFI_SCANNING,
    WIFI_INIT_FAIL,
    WIFI_AUTHENTICATED_FAIL,	//WEP加密失败
    WIFI_ASSOCIATED_FAIL,		//关联错误
    WIFI_AUTH_FAIL,				//WPA加密认证失败
} WIFISTASTATUS;

typedef enum
{
    PORT_WAN                = 0,
    PORT_LAN1           	= 1,
    PORT_LAN2  				= 2,
    PORT_LAN3     			= 3,
    PORT_LAN4           	= 4,
} PORTS;

typedef enum
{
	WL_OFF = 0,
    WL_ON,
    WL_WATING,
} WL_RADIO_TYPE;

#ifdef __CONFIG_PPPOE_SERVER__
typedef enum{
	SET_SYBCHRO_TIME = 0,
	GET_SYBCHRO_TIME
}SYBCHRO_TIME;

 typedef enum{
	AUTO_SYNCHRO = 0,
	MANUAL_SYNCHRO
}SYNCHRO_TYPE;

  typedef enum{
	SYNCHROING = 0,
	SYNCHRO_SUCESS
}SYNCHRO_STATUS;

typedef enum wlan_rate_type{
			WLAN_RATE_5G=5,
			WLAN_RATE_24G=24,
			WLAN_RATE_ALL=29  // all bands, for service operation
}WLAN_RATE_TYPE;

extern void set_synchro_status(SYNCHRO_STATUS type);
extern SYNCHRO_STATUS get_synchro_status();
extern unsigned long long synchro_config_start_time(unsigned long long time,int operate);
extern SYNCHRO_TYPE get_synchro_type();
extern void set_synchro_type(SYNCHRO_TYPE type);
#endif
#endif/*__PI_COMMON_H__*/
