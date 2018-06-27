#ifndef __UC_M_RUB_NET_TYPES_H__
#define __UC_M_RUB_NET_TYPES_H__

#define	DEVICE_NAME_LEN		64
#define	MAC_STRING_LEN		20
#define	TIME_LEN			16
#define	ASSO_SSID_LEN		36
#define	ASSO_SN_LEN			20

#define	HISTORY_LIST_MAX	20
#define	BLACK_LIST_MAX	    30



#define SET_RUB_NET_X(net,x) \
	((net)->mask |= (1 << x))
#define HAS_RUB_NET_X(net,x) \
	(((net)->mask & (1 << x)) == (1 << x))

typedef enum {
	RUB_NET_OPT_ADD_TO_BLACKLIST = 0,	/*加入黑名单*/
	RUB_NET_OPT_ADD_TO_TRUSTLIST,	/*加入信任列表*/
	RUB_NET_OPT_RM_FROM_TRUSTLIST,	/*从信任列表中删除*/
	RUB_NET_OPT_RM_FROM_BLACKLIST,	/*从黑名单中删除*/
}rub_net_opt_t;

typedef struct access_user_s {
	char	mac[MAC_STRING_LEN];
	rub_net_opt_t		op;	
}access_user_t;

typedef struct rub_net_s {
	int		status;
}rub_net_t;

typedef struct history_list_s {
	char	dev_name[DEVICE_NAME_LEN];
	char	mac[MAC_STRING_LEN];
	char	add_date[TIME_LEN];
	char	add_time[TIME_LEN];
}history_list_t;

typedef	struct rub_network_history_s {
	int		n_history;
	history_list_t	user[HISTORY_LIST_MAX];
}rub_network_history_t;

typedef struct rub_strange_host_info_s {
	char serial_num[ASSO_SN_LEN];
	char	mac[MAC_STRING_LEN];		
	char dev_name[DEVICE_NAME_LEN];
	int dtype;				//主机类型
	int type;				//消息类型
	char	date[TIME_LEN];		//2015-01-03
	char	time[TIME_LEN];		//12:02
	char	ssid[ASSO_SSID_LEN];		//only copy 32
}rub_strange_host_info_t;
//
typedef struct black_user_s {
	char	mac[MAC_STRING_LEN];
	char    dev_name[DEVICE_NAME_LEN];
}black_user_t;

typedef struct black_list_s {
	int 	     n_mac;
	black_user_t buser[0];
}black_list_t;

typedef struct black_list_s white_list_t;
enum{
	_MACFILTER_ENABLE = 0,
	_MACFILTER_MODE, 
	_MACFILTER_SUPT_MODE,
};
#define HAS_MACFILTER_X(mf, x) \
		((mf)->mask&(1<<x))
#define SET_MACFILTER_X(mf, x) \
		((mf)->mask |= (1<<x))
#define HAS_MACFILTER_MODE(mf) \
		HAS_MACFILTER_X(mf, _MACFILTER_MODE)
#define HAS_MACFILTER_ENABLE(mf) \
		HAS_MACFILTER_X(mf, _MACFILTER_ENABLE)
#define HAS_MACFILTER_SUPT_MODE(mf) \
		HAS_MACFILTER_X(mf, _MACFILTER_SUPT_MODE)
		
#define SET_MACFILTER_MODE(mf) \
		SET_MACFILTER_X(mf, _MACFILTER_MODE)
#define SET_MACFILTER_ENABLE(mf) \
		SET_MACFILTER_X(mf, _MACFILTER_ENABLE)
#define SET_MACFILTER_SUPT_MODE(mf) \
		SET_MACFILTER_X(mf, _MACFILTER_SUPT_MODE)
		
		
typedef struct mac_filter_mode_s{
	int mask;
	int enable;
	int mac_mode;
	int supt_mac_mode;
}mac_filter_mode_t;
typedef struct access_list_set{
	int 	n_mac;
	char	mac[BLACK_LIST_MAX][MAC_STRING_LEN];
}access_list_set_t;

enum {
	_RUB_NET_ACK_HISTORY = 0,
	_RUB_NET_ACK_STATUS,
	_RUB_NET_ACK_BLACKLIST,
	_RUB_NET_ACK_GLOBAL_MF,
};

#define SET_RUB_NET_ACK_HISTORY(ack) \
	SET_RUB_NET_X(ack,_RUB_NET_ACK_HISTORY)
#define SET_RUB_NET_ACK_STATUS(ack) \
	SET_RUB_NET_X(ack,_RUB_NET_ACK_STATUS)
#define SET_RUB_NET_ACK_BLACKLIST(ack) \
	SET_RUB_NET_X(ack,_RUB_NET_ACK_BLACKLIST)
#define SET_RUB_NET_ACK_GLOBAL_MF(ack) \
	SET_RUB_NET_X(ack,_RUB_NET_ACK_GLOBAL_MF)	
#define HAS_RUB_NET_ACK_HISTORY(ack) \
	HAS_RUB_NET_X(ack,_RUB_NET_ACK_HISTORY)
#define HAS_RUB_NET_ACK_STATUS(ack) \
	HAS_RUB_NET_X(ack,_RUB_NET_ACK_STATUS)
#define HAS_RUB_NET_ACK_BLACKLIST(ack) \
	HAS_RUB_NET_X(ack,_RUB_NET_ACK_BLACKLIST)	
#define HAS_RUB_NET_ACK_GLOBAL_MF(ack) \
	HAS_RUB_NET_X(ack,_RUB_NET_ACK_GLOBAL_MF)

typedef struct rub_net_common_ack_s {
	int mask;
	int err_code;
}rub_net_common_ack_t;

#endif
