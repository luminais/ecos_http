#include <bcmcrypto/rc4.h>
#include <net/ethernet.h>

#ifdef __CONFIG_EXTEND_LED__
#include "sys_extend_timer.h"
#endif

#ifdef __CONFIG_EXTEND_LED__
#include "sys_extend_timer.h"
#endif

#include "sys_module.h"
#include "rc_module.h"

#include "shutils.h"//包含strcat_r函数的定义

#define KEY_STR_LEN 18

#define ETH_HEAD_LEN 	(14)
#define PORT_ATUO_CONN 	0x1234

#define CONF_BUF_LEN 1024


#define nvram_recv_safe_get(name) (nvram_get(name) ? : "")

enum wireless_pass
{
	NONE,
	WEP,
	WPA_AES,
	WPA_TKIP,
	WPA2_AES,
	WPA2_TKIP,
	WPA_WPA2_AES,
	WPA_WPA2_TKIP,
	WPA_WPA2_AES_TKIP
};

#pragma pack(1)
typedef struct data_pkt
{
	unsigned int rnd;
	unsigned int ack;
	int  dataLen;
	unsigned char *data;
} auto_conn_pkt;

typedef struct passwordtype
{
	unsigned int pass_index;
	void (*p_password)(struct transmit_wifi_info *);
}PASSWORDTYPE;
#define SELECT_TYPE(node)  ((node).p_password)
struct transmit_wifi_info
{
	unsigned int enTpye;
	unsigned int channel;
	char ssid[33];
	char passwd[65];
};
#pragma pack()
enum CONNECT_STATUS
{
	AUTO_CONN_VIF_5G = 5,
	AUTO_CONN_VIF_24G = 24
};
#ifdef __CONFIG_AUTO_CONN_CLIENT__
enum AUTO_CONN_EXTEND_STATUS
{
	AUTO_CONN_VIF_EXTEND_UNDO,
	AUTO_CONN_VIF_EXTEND_SETDEF,
	AUTO_CONN_VIF_EXTEND_DOING,
	AUTO_CONN_VIF_EXTEND_DONE
};
#endif
typedef struct client_state
{
	int state;
	union
	{
		void (*p_doing)(struct ifnet *,struct ifnet *);
	}action;
}CLIENT_STATE;
#define DO_DOING(node) (((node).action).p_doing)

#ifdef __CONFIG_AUTO_CONN_SERVER__
enum Auto_vif_extend_start
{
	AUTO_CONN_VIF_ROUTER_UNDO,
	AUTO_CONN_VIF_ROUTER_INIT,
	AUTO_CONN_VIF_ROUTER_DOING
};

#define FLUSH_TIME		13
#define TURN_ON_VIR_IF 	12
#define TURN_OFF_VIR_IF	11
#define AUTO_CONNECT_SERVER_STOP 	14

#endif

#define AUTO_CONN_DEBUG_OFF 0
#define AUTO_CONN_DEBUG_ERR 1

#define AUTO_CONN_DBGPRINT(Level, fmt, args...)   \
	{                                           \
	    if (Level <= AutoConnDebugLevel)                      \
	    {                                       \
			printf( fmt, ## args);         \
	    }                                       \
	}

