#ifndef _PI_COMMON_H_
#define _PI_COMMON_H_

#define MAX_IP_LEN 17

#define CON_COLOR_END               "\033[m"
#define CON_COLOR_GREEN             "\033[1;32m"
#define CON_COLOR_RED               "\033[1;31m"
#define CON_COLOR_PURPLE            "\033[1;35m"
#define CON_COLOR_YELLOW            "\033[1;33m"
#define CON_COLOR_CYAN              "\033[1;36m"

#define GPI "GPI"

#define TPI "TPI"

#define API "API"

#define PI_ERROR(MODULE,fmt, arg...) \
        do {printf(CON_COLOR_RED "[%s >>%s(%d)]:" CON_COLOR_END fmt, MODULE , __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_WARNING(MODULE,fmt, arg...) \
        do {printf(CON_COLOR_PURPLE "[%s >>%s(%d)]:" CON_COLOR_END fmt,MODULE, __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_DEBUG(MODULE,fmt, arg...) \
        do {printf(CON_COLOR_GREEN "[%s >>%s(%d)]:" CON_COLOR_END fmt,MODULE, __FUNCTION__, __LINE__, ##arg); } while (0)

#define PI_PRINTF(MODULE,fmt, arg...) \
        do { printf(CON_COLOR_RED "[%s >>%s(%d)]:" CON_COLOR_END fmt,MODULE, __FUNCTION__, __LINE__, ##arg); } while (0)

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) > (b) ? (b) : (a))
#endif
		
typedef enum
{
    PI_ERR        = 0,
    PI_SUC        = 1,
} PI_INFO;

typedef enum
{
    PI_STOP        		= 0,
    PI_START        	= 1,
    PI_RESTART        	= 2,
} PI_ACTION;

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
    WL_WISP_MODE            = 1,
    WL_APCLIENT_MODE        = 2,
    WL_WDS_MODE             = 3,
} WLMODE;

typedef enum
{
    WAN_NONE_MODE           = 0,
	WAN_STATIC_MODE 		= 1,
    WAN_DHCP_MODE           = 2,
    WAN_PPPOE_MODE          = 3,
} WANMODE;

typedef enum
{
    NETWORK_CHECK_NONE_MODE           = 0,
	NETWORK_CHECK_STATIC_MODE         = 1,
    NETWORK_CHECK_DHCP_MODE           = 2,
    NETWORK_CHECK_PPPOE_MODE          = 3,
} NETWORKCHECKMODE;

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
    WIFI_INIT_FAIL           = 1,
	WIFI_ASSOCIATED_FAIL	 = 2,//关联错误
	WIFI_AUTHENTICATED_FAIL  = 3,//WEP加密失败
    WIFI_AUTH_FAIL  		 = 4,//WPA加密认证失败
} WIFISTASTATUS;

typedef enum
{
    PORT_WAN                = 0,
    PORT_LAN1           	= 1,
    PORT_LAN2  				= 2,
    PORT_LAN3     			= 3,
    PORT_LAN4           	= 4,
} PORTS;
#endif
