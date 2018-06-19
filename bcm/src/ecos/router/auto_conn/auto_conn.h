#include <bcmcrypto/rc4.h>
#include <net/ethernet.h>
#define KEY_STR_LEN 18

#define ETH_HEAD_LEN 	(14)
#define PORT_ATUO_CONN 	0x1234

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
typedef struct data_pkt{
	unsigned int rnd;
	unsigned int ack;
	int  dataLen;
	unsigned char *data;
}auto_conn_pkt;

struct wifi_info
{
	unsigned int enTpye;
	unsigned int channel;
	char ssid[33];
	char passwd[65];
};
#pragma pack()

enum Auto_vif_extend_start
{
	AUTO_CONN_VIF_EXTEND_UNDO,
	AUTO_CONN_VIF_EXTEND_INIT,
	AUTO_CONN_VIF_EXTEND_DOING
};

#define FLUSH_TIME		13
#define TURN_ON_VIR_IF 	12 
#define TURN_OFF_VIR_IF	11

#define AUTO_CONN_DEBUG_OFF 0
#define AUTO_CONN_DEBUG_ERR 1

#define AUTO_CONN_DBGPRINT(Level, fmt, args...)   \
	{                                           \
	    if (Level <= AutoConnDebugLevel)                      \
	    {                                       \
			diag_printf( fmt, ## args);         \
	    }                                       \
	}
