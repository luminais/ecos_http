/*
**  Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
**
**  Project:	Cloud manager Api v1.0
**  File:    	
**  Author: 	xiongping
**  Date:    	07/23/2015
**
**  Purpose:
**    		.
**
**  History: 
**  <author>   <time>          <version >   <desc>
*	$Id: 
*/

#ifndef __UC_M_SAFE_CHECK_TYPES_H__
#define __UC_M_SAFE_CHECK_TYPES_H__
#include <stdint.h>
#define SET_SAFE_CHECK_X(safe,x) \
	((safe)->mask |= (1 << x))
#define HAS_SAFE_CHECK_X(safe,x) \
	(((safe)->mask & (1 << x)) == (1 << x))

enum {
	_SAFE_CHECK_INFO_WIFI_50_PWD_ = 0,
	_SAFE_CHECK_INFO_DDOS_ATTACK_,
};
#define SET_SAFE_CHECK_INFO_WIFI_50_PWD(safe) \
	SET_SAFE_CHECK_X(safe,_SAFE_CHECK_INFO_WIFI_50_PWD_)
#define SET_SAFE_CHECK_INFO_DDOS_ATTACK(safe) \
	SET_SAFE_CHECK_X(safe,_SAFE_CHECK_INFO_DDOS_ATTACK_)
#define HAS_SAFE_CHECK_INFO_WIFI_50_PWD(safe) \
	HAS_SAFE_CHECK_X(safe,_SAFE_CHECK_INFO_WIFI_50_PWD_) 
#define HAS_SAFE_CHECK_INFO_DDOS_ATTACK(safe) \
	HAS_SAFE_CHECK_X(safe,_SAFE_CHECK_INFO_DDOS_ATTACK_) 
	
typedef struct safe_check_info_s {
	int mask;
	int auth_pwd_sta;                       /*web登录密码检测结果*/
	int wifi_24_pwd_sta;                 /*wifi 2.4g密码的检测结果*/
	int wifi_50_pwd_sta;                 /*wifi 5g密码的检测结果*/
	int dns_hijack_sta;                    /*是否有dns劫持*/
	int ddos_attack_sta;                 /*是否有ddos攻击*/
}safe_check_info_t;

typedef struct guard_info_s{
	uint32_t login_anti_crack;
	uint32_t wl_anti_crack;
	uint32_t dns_anti_hijack;
	uint32_t sys_anti_attack;
}guard_info_t;

// safe check common ack
enum {
	_SAFE_CHECK_ACK_INFO_ = 0,
	_SAFE_GUARD_ACK_INFO_,
};
#define SET_SAFE_CHECK_ACK_INFO(safe) \
	SET_SAFE_CHECK_X(safe,_SAFE_CHECK_ACK_INFO_)
#define HAS_SAFE_CHECK_ACK_INFO(safe) \
	HAS_SAFE_CHECK_X(safe,_SAFE_CHECK_ACK_INFO_) 
	
#define SET_SAFE_GUARD_ACK_INFO(safe) \
	SET_SAFE_CHECK_X(safe,_SAFE_GUARD_ACK_INFO_)
#define HAS_SAFE_GUARD_ACK_INFO(safe) \
	HAS_SAFE_CHECK_X(safe,_SAFE_GUARD_ACK_INFO_) 
	
typedef struct safe_check_common_ack_s {
	int mask;
	int err_code;
	safe_check_info_t result;
	guard_info_t guard_detail;
}safe_check_common_ack_t;

#endif

