/*
**  Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
**
**  Project:	Cloud manager Api v1.0
**  File:    	
**  Author: 	lixiaolong
**  Date:    	05/14/2015
**
**  Purpose:
**    		.
**
**  History: 
**  <author>   <time>          <version >   <desc>
*	$Id: 
*/

#ifndef __UC_M_WIFI_TYPES_H__
#define __UC_M_WIFI_TYPES_H__
#include <stdint.h>

#define MAX_SSID_LENGTH				64
#define MAX_WIFI_PASSWD_LENGTH 		64 
#define MAX_WIFI_SEC_LENGTH 		64 
#define  BUFF_LEN   16

#define MAX_SEC_OPTION 		5
#define MAX_INT_OPTION		10

//common mask set and has
#define SET_WIFI_X(wifi, x)	\
	((wifi)->mask |= (1 << x))
#define HAS_WIFI_X(wifi, x)	\
	(((wifi)->mask & (1 << x)) == (1 << x))
/*
** wifi struct
*/
typedef enum {
	WIFI_2G = 0,
	WIFI_5G,
	WIFI_MAX,
}wifi_type_t;

enum {
	WIFI_BASIC_DETAIL_SEC = 0,
	WIFI_BASIC_DETAIL_ENABLE,
	WIFI_BASIC_DETAIL_PASSWD,
	WIFI_BASIC_DETAIL_SSID_HIDE
};

#define SET_WIFI_BASIC_2G(wifi_basic) \
	SET_WIFI_X(wifi_basic, WIFI_2G)
#define SET_WIFI_BASIC_5G(wifi_basic) \
	SET_WIFI_X(wifi_basic, WIFI_5G)
#define HAS_WIFI_BASIC_2G(wifi_basic) \
	HAS_WIFI_X(wifi_basic, WIFI_2G)
#define HAS_WIFI_BASIC_5G(wifi_basic)  \
	HAS_WIFI_X(wifi_basic, WIFI_5G)

#define SET_WIFI_BASIC_DETAIL_SEC(wifi_basic_detail) \
	SET_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_SEC)
#define SET_WIFI_BASIC_DETAIL_PASSWD(wifi_basic_detail) \
	SET_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_PASSWD)
#define SET_WIFI_BASIC_DETAIL_SSID_HIDE(wifi_basic_detail) \
	SET_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_SSID_HIDE)	
#define SET_WIFI_BASIC_DETAIL_ENABLE(wifi_basic_detail) \
	SET_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_ENABLE)	
	
#define HAS_WIFI_BASIC_DETAIL_SEC(wifi_basic_detail) \
	HAS_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_SEC)
#define HAS_WIFI_BASIC_DETAIL_PASSWD(wifi_basic_detail) \
	HAS_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_PASSWD)
#define HAS_WIFI_BASIC_DETAIL_SSID_HIDE(wifi_basic_detail) \
	HAS_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_SSID_HIDE)
#define HAS_WIFI_BASIC_DETAIL_ENABLE(wifi_basic_detail) \
	HAS_WIFI_X(wifi_basic_detail, WIFI_BASIC_DETAIL_ENABLE)


typedef struct wifi_detail_s {
	int 	mask;
	wifi_type_t 	type;
	int	enable;
	char	ssid[MAX_SSID_LENGTH];
	char	passwd[MAX_WIFI_PASSWD_LENGTH];
	int		ssid_hide;
	char sec[MAX_WIFI_SEC_LENGTH];	
} wifi_detail_t;

//wifi basic info
typedef struct wifi_basic_s {
	int 			mask;
	int 			n_wifi_detail;
	wifi_detail_t	wifi_detail[WIFI_MAX];
	uint32_t 	 n_sec_option;
	char		 sec_option[MAX_SEC_OPTION][MAX_WIFI_SEC_LENGTH];
} wifi_basic_t;


/*
 *WIFI GUEST INFO
 */
enum {
	WIFI_GUEST_DETAIL_SEC = 0,
	WIFI_GUEST_DETAIL_PWD
};

enum{
	WIFI_GUEST_TIMEOUT = WIFI_MAX,
	WIFI_GUEST_RATE,
	WIFI_GUEST_RATE_UPLIMIT,
};

#define SET_WIFI_GUEST_2G(guest_info) \
	SET_WIFI_X(guest_info, WIFI_2G)
#define SET_WIFI_GUEST_5G(guest_info) \
	SET_WIFI_X(guest_info, WIFI_5G)
#define HAS_WIFI_GUEST_2G(guest_info) \
	HAS_WIFI_X(guest_info, WIFI_2G)
#define HAS_WIFI_GUEST_5G(guest_info)  \
	HAS_WIFI_X(guest_info, WIFI_5G)

#define HAS_WIFI_GUEST_TIMEOUT(guest_info) \
	HAS_WIFI_X(guest_info, WIFI_GUEST_TIMEOUT)

#define HAS_WIFI_GUEST_RATE(guest_info) \
        HAS_WIFI_X(guest_info, WIFI_GUEST_RATE)
		
#define HAS_WIFI_GUEST_RATE_UPLIMIT(guest_info) \
		HAS_WIFI_X(guest_info, WIFI_GUEST_RATE_UPLIMIT)

#define SET_WIFI_GUEST_TIMEOUT(guest_info) \
	SET_WIFI_X(guest_info, WIFI_GUEST_TIMEOUT)

#define SET_WIFI_GUEST_RATE(guest_info) \
    	SET_WIFI_X(guest_info, WIFI_GUEST_RATE)
	
#define SET_WIFI_GUEST_RATE_UPLIMIT(guest_info) \
		SET_WIFI_X(guest_info, WIFI_GUEST_RATE_UPLIMIT)


#define SET_WIFI_GUEST_DETAIL_SEC(guest_detail) \
	SET_WIFI_X(guest_detail, WIFI_GUEST_DETAIL_SEC)
#define SET_WIFI_GUEST_DETAIL_PWD(guest_detail) \
	SET_WIFI_X(guest_detail, WIFI_GUEST_DETAIL_PWD)
#define HAS_WIFI_GUEST_DETAIL_SEC(guest_detail) \
	HAS_WIFI_X(guest_detail, WIFI_GUEST_DETAIL_SEC)
#define HAS_WIFI_GUEST_DETAIL_PWD(guest_detail) \
	HAS_WIFI_X(guest_detail, WIFI_GUEST_DETAIL_PWD)

typedef struct guest_detail_s {
	int 	mask;
	wifi_type_t type;
	int 	guest_enable;
	char	guest_ssid[MAX_SSID_LENGTH];	
	char	guest_passwd[MAX_WIFI_PASSWD_LENGTH];
	char 	sec[MAX_WIFI_SEC_LENGTH];
}guest_detail_t;


typedef struct guest_info_s {
	int    	mask;
	guest_detail_t guest_detail[WIFI_MAX];
	uint32_t timeout;
	uint32_t rate;
	uint32_t rate_uplimit;
	uint32_t n_timeout_option;
	int32_t	 timeout_option[MAX_INT_OPTION]; 
	uint32_t n_rate_option;
	int32_t  rate_option[MAX_INT_OPTION];
} guest_info_t;

/*
 *WIFI CHANNEL
 */
 enum {
	WIFI_CHANNEL_5G = 0,
};

#define SET_WIFI_CHANNEL_5G(wifi_channel) \
	SET_WIFI_X(wifi_channel, WIFI_CHANNEL_5G)
#define HAS_WIFI_CHANNEL_5G(wifi_channel) \
	HAS_WIFI_X(wifi_channel, WIFI_CHANNEL_5G)
	
typedef struct wifi_channel_info_s {
	int mask;
	int 	chan_2g_sta;
	int 	chan_5g_sta;
} wifi_channel_info_t;

/*
 * WIFI POWER
 */
  enum {
	WIFI_POWER_5G = 0,
};

#define SET_WIFI_POWER_5G(wifi_power) \
	SET_WIFI_X(wifi_power, WIFI_POWER_5G)
#define HAS_WIFI_POWER_5G(wifi_power) \
	HAS_WIFI_X(wifi_power, WIFI_POWER_5G)	
	
typedef struct wifi_power_s {
	int 	mask;
	int		wifi_2g_power;
	int		wifi_5g_power;
} wifi_power_t;

/*
*WIFI COMMON ACK
*/
enum {
	_WIFI_BASIC_ = 0,
	_WIFI_GUEST_,
	_WIFI_CHANNEL_,
	_WIFI_POWER_,
};

#define SET_ACK_WIFI_BASIC(ack) \
	SET_WIFI_X(ack, _WIFI_BASIC_)
#define SET_ACK_WIFI_GUEST(ack) \
	SET_WIFI_X(ack, _WIFI_GUEST_)
#define SET_ACK_WIFI_CHANNEL(ack) \
	SET_WIFI_X(ack, _WIFI_CHANNEL_)
#define SET_ACK_WIFI_POWER(ack) \
	SET_WIFI_X(ack, _WIFI_POWER_)
#define HAS_ACK_WIFI_BASIC(ack) \
	HAS_WIFI_X(ack, _WIFI_BASIC_)
#define HAS_ACK_WIFI_GUEST(ack) \
	HAS_WIFI_X(ack, _WIFI_GUEST_)
#define HAS_ACK_WIFI_CHANNEL(ack) \
	HAS_WIFI_X(ack, _WIFI_CHANNEL_)
#define HAS_ACK_WIFI_POWER(ack) \
	HAS_WIFI_X(ack, _WIFI_POWER_)
	
typedef struct wifi_common_ack_s {
	int mask;
	int err_code;		//cmd execute error, defines by register process
	wifi_basic_t	basic;
	guest_info_t	guest;
	wifi_channel_info_t channel;
	wifi_power_t	power;
}wifi_common_ack_t;

#endif

