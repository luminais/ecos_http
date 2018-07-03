#ifndef __UC_M_WAN_TYPES_H__
#define __UC_M_WAN_TYPES_H__

//#define ADSL_NAME	32
//#define ADSL_PWD 	32
#define ADSL_NAME	256
#define ADSL_PWD 	256
#define ANY_TP_NAME	32		//l2tp or pptp
#define ANY_TP_PWD 	32
#define WAN_NUM		5
#define TEMP_LEN		16

//common mask set and has
#define SET_WAN_X(wan, x)	\
	((wan)->mask |= (1 << x))
#define HAS_WAN_X(wan, x)	\
	(((wan)->mask & (1 << x)) == (1 << x))

//basic info	
typedef enum NETWORKTYPE {
	ADSL = 1,
	DYNAMIC,
	STATIC,
	L2TP,
	PPTP,
	DOUBLE_ADSL,
}E_NETWORK_T;

typedef struct adsl_info_s{
	char adsl_name[ADSL_NAME];
	char adsl_pwd[ADSL_PWD];
}adsl_info_t;

enum{
	_WAN_CONN_TIME = 0,
};
#define SET_WAN_CONN_TIME(net)	\
	SET_WAN_X(net, _WAN_CONN_TIME)
#define HAS_WAN_CONN_TIME(net)	\
	HAS_WAN_X(net, _WAN_CONN_TIME)
	
typedef struct net_addr_info_s{
	unsigned int mask;
	unsigned int ip_addr;
	unsigned int netmask;
	unsigned int gateway;
	unsigned int primary_dns;
	unsigned int backup_dns;
	int 		 conn_time;
}net_addr_info_t;

enum {
	_WAN_NET_TP_ADDR = 0,
};
#define SET_WAN_NET_TP_ADDR(tp)	\
	SET_WAN_X(tp, _WAN_NET_TP_ADDR)
#define HAS_WAN_NET_TP_ADDR(tp)	\
	HAS_WAN_X(tp, _WAN_NET_TP_ADDR)
	
typedef struct net_tp_info_s {
	int mask;
	unsigned int svr_ip_addr;	//服务器地址
	char any_tp_name[ANY_TP_NAME];
	char any_tp_pwd[ANY_TP_PWD];
	int mppe_en;		//是否启用mppe加密
	E_NETWORK_T work_mode;	//only use dynamic or static
//	net_addr_info_t 	netaddr_info;
}net_tp_info_t;

typedef struct wan_status_s{
	int   sta;                
	int  err;
}wan_status_t;

enum {
	_WAN_BASIC_WAN_STA = 0,
	_WAN_BASIC_WAN_ADSL_INFO,
	_WAN_BASIC_WAN_NET_ADDR_INFO,
	_WAN_BASIC_WAN_NET_TP_INFO,
};
#define SET_WAN_BASIC_WAN_STA(basic)	\
	SET_WAN_X(basic, _WAN_BASIC_WAN_STA)
#define SET_WAN_BASIC_WAN_ADSL_INFO(basic)	\
	SET_WAN_X(basic, _WAN_BASIC_WAN_ADSL_INFO)
#define SET_WAN_BASIC_WAN_NET_ADDR_INFO(basic)	\
	SET_WAN_X(basic, _WAN_BASIC_WAN_NET_ADDR_INFO)
#define SET_WAN_BASIC_WAN_NET_TP_INFO(basic)	\
	SET_WAN_X(basic, _WAN_BASIC_WAN_NET_TP_INFO)
#define HAS_WAN_BASIC_WAN_STA(basic)	\
	HAS_WAN_X(basic, _WAN_BASIC_WAN_STA)
#define HAS_WAN_BASIC_WAN_ADSL_INFO(basic)	\
	HAS_WAN_X(basic, _WAN_BASIC_WAN_ADSL_INFO)
#define HAS_WAN_BASIC_WAN_NET_ADDR_INFO(basic)	\
	HAS_WAN_X(basic, _WAN_BASIC_WAN_NET_ADDR_INFO)
#define HAS_WAN_BASIC_WAN_NET_TP_INFO(basic)	\
	HAS_WAN_X(basic, _WAN_BASIC_WAN_NET_TP_INFO)
	
typedef struct wan_basic_detail_s{
	int mask;
	int				 interfacenum;	//detail which wan
	wan_status_t		 wan_status;
	E_NETWORK_T		 type;
	adsl_info_t 		adsl_info;
	net_addr_info_t 	netaddr_info;
	net_tp_info_t		nettp_info;
}wan_basic_detail_t;


typedef struct wan_info_s{
	int mask;
	int n_wan;
	wan_basic_detail_t		wan[WAN_NUM];
}wan_basic_info_t;

//rate info
typedef struct wan_rate_detail_s {
	int interfacenum;
	int cur_uplink;			//当前实时速率
	int cur_downlink;
	int max_uplink;		//最大值,测速所得或者配置
	int max_downlink;
}wan_rate_detail_t;

typedef struct wan_rate_info_s{
	int mask;
	int n_wan;
	wan_rate_detail_t wan[WAN_NUM];
}wan_rate_info_t;


//detect type 
typedef enum DETECT_TYPE {
	NO_LINE = -2,
	DETECTING  = -1,
	DET_DHCP = 0,
	DET_STATIC= 1,
	DET_PPPOE = 2	
}DETECT_TYPE_T;

typedef struct wan_detecttype_detail_s{
	DETECT_TYPE_T detect_type;
}wan_detecttype_detail_t;
typedef struct wan_detecttype_info_s{
	int n_wan;
	wan_detecttype_detail_t wan[WAN_NUM];
}wan_detecttype_info_t;

//common ack
enum {
	_WAN_ACK_BASIC = 0,
	_WAN_ACK_RATE,
	_WAN_ACK_DETECT_TYPE,
};
#define SET_WAN_ACK_BASIC(ack)	\
	SET_WAN_X(ack, _WAN_ACK_BASIC)
#define SET_WAN_ACK_RATE(ack)	\
	SET_WAN_X(ack, _WAN_ACK_RATE)
#define SET_WAN_ACK_DETECT_TYPE(ack)	\
	SET_WAN_X(ack, _WAN_ACK_DETECT_TYPE)
#define HAS_WAN_ACK_BASIC(ack)	\
	HAS_WAN_X(ack, _WAN_ACK_BASIC)
#define HAS_WAN_ACK_RATE(ack)	\
	HAS_WAN_X(ack, _WAN_ACK_RATE)
#define HAS_WAN_ACK_DETECT_TYPE(ack)	\
	HAS_WAN_X(ack, _WAN_ACK_DETECT_TYPE)
typedef struct wan_common_ack_s {
	int mask;
	int err_code;
	wan_basic_info_t basic_info;
	wan_rate_info_t rate_info;
	wan_detecttype_info_t type_info;
}wan_common_ack_t;

#endif
