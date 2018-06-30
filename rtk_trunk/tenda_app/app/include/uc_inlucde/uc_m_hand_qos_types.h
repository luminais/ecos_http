/*
**  Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
**
**  Project:	Cloud manager Api v1.0
**  File:    	
**  Author: 	yangyabing
**  Date:    	07/09/2015
**
**  Purpose:
**    		.
**
**  History: 
**  <author>   <time>          <version >   <desc>
*	$Id: 
*/

#ifndef __M_HAND_QOS_TYPES_H__
#define __M_HAND_QOS_TYPES_H__
//#include "public_types.h"
#define HAND_QOS_MAC_STR_LEN			20
#define QOS_MAC_STR_LEN			20
#define QOS_IP_STR_LEN				16	
#define DEV_NAME_STR_LEN                                64

#define SET_HAND_QOS_X(qos,x) \
	((qos)->mask |= (1 << x))
#define HAS_HAND_QOS_X(qos,x) \
	(((qos)->mask & (1 << x)) == (1 << x))
	
typedef enum {
	HAND_QOS_OPT_GET_ALL = 0,
	HAND_QOS_OPT_GET_SPEC,
}hand_qos_get_param_opt;

typedef enum {
	HAND_QOS_OPT_SET_ADD = 0,
	HAND_QOS_OPT_SET_UPDATE,
	HAND_QOS_OPT_SET_DELETE,
}hand_qos_set_param_opt;

typedef enum {
	HAND_QOS_ENABLE = 0,
	HAND_QOS_DEV_NAME,
	HAND_QOS_UP_RATE,
	HAND_QOS_DOWN_RATE,
	HAND_QOS_SHARE_MODE,
	HAND_QOS_POLICY_MODE,
	HAND_QOS_PRIORITY_ENABLE,
}__hand_qos_rule_param_t;

#define SET_HAND_QOS_ENABLE(hand_qos_rule_t)	\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_ENABLE)
#define SET_HAND_QOS_DEV_NAME(hand_qos_rule_t)	\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_DEV_NAME)
#define SET_HAND_QOS_UP_RATE(hand_qos_rule_t)	\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_UP_RATE)
#define SET_HAND_QOS_DOWN_RATE(hand_qos_rule_t)	\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_DOWN_RATE)
#define SET_HAND_QOS_SHARE_MODE(hand_qos_rule_t)	\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_SHARE_MODE)
#define SET_HAND_QOS_POLICY_MODE(hand_qos_rule_t)		\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_POLICY_MODE)
#define SET_HAND_QOS_PRIORITY_ENABLE(hand_qos_rule_t)		\
	SET_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_PRIORITY_ENABLE)

#define HAS_HAND_QOS_ENABLE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_ENABLE)
#define HAS_HAND_QOS_DEV_NAME(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_DEV_NAME)
#define HAS_HAND_QOS_UP_RATE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_UP_RATE)
#define HAS_HAND_QOS_DOWN_RATE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_DOWN_RATE)
#define HAS_HAND_QOS_SHARE_MODE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_SHARE_MODE)
#define HAS_HAND_QOS_POLICY_MODE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_POLICY_MODE)
#define HAS_HAND_QOS_PRIORITY_ENABLE(hand_qos_rule_t) \
	HAS_HAND_QOS_X(hand_qos_rule_t,HAND_QOS_PRIORITY_ENABLE)
	
typedef enum {
	HAND_QOS_GET_PARAM_MAC = 0,
}__hand_qos_get_param_t;

#define SET_HAND_QOS_GET_PARAM_MAC(hand_qos_get_param_t)	\
	SET_HAND_QOS_X(hand_qos_get_param_t,HAND_QOS_GET_PARAM_MAC)
#define HAS_HAND_QOS_GET_PARAM_MAC(hand_qos_get_param_t) \
	HAS_HAND_QOS_X(hand_qos_get_param_t , HAND_QOS_GET_PARAM_MAC)

	
typedef struct hand_qos_rule_s {
	int mask;
	int enable;								//qos规则是否启用
	char mac_addr[QOS_MAC_STR_LEN];			//传给qos规则中对应的mac地址
	char dev_name[DEV_NAME_STR_LEN];                   //dev name
	float u_rate;								//上行限速
	float d_rate;								//下行限速      5.00KB/S
	int share_mode;							//共享模式		no use	
	int policy_mode;							//策略模式		no use
	int priority_enable;							//优先级启用		no use
}hand_qos_rule_t;

typedef struct hand_qos_info_s {
	int rule_count;
	int mem_len;                     //柔性数组真实的存储大小
	hand_qos_rule_t rules[0];
}hand_qos_info_t;

typedef struct hand_qos_get_param_s {
	int mask;
	hand_qos_get_param_opt opt;
	char mac[QOS_MAC_STR_LEN]; 
}hand_qos_get_param_t;

typedef struct hand_qos_set_param_s {
	int mask;
	hand_qos_set_param_opt opt;
	hand_qos_rule_t rule; 
}hand_qos_set_param_t;

typedef struct hand_qos_set_info_s{
	int rule_count;
	int mem_len;                        //柔性数组真实的存储大小
	hand_qos_set_param_t set_rules[0];
}hand_qos_set_info_t;

enum{
	_HAND_QOS_SET_PARAM_OPT = 0,
};
#define SET_HAND_QOS_SET_PARAM_OPT(hand_qos_set_param_t)		\
	SET_HAND_QOS_X(hand_qos_set_param_t,_HAND_QOS_SET_PARAM_OPT)

#define HAS_HAND_QOS_SET_PARAM_OPT(hand_qos_set_param_t) \
	HAS_HAND_QOS_X(hand_qos_set_param_t,_HAND_QOS_SET_PARAM_OPT)

typedef struct hand_qos_global_en_s {
	int global_en;
}hand_qos_global_en_t;

enum{
	_HAND_LOWER_LIMIT_UP,
	_HAND_LOWER_LIMIT_DOWN,
};

#define HAS_LOWER_LIMITE_UP(param) \
	HAS_HAND_QOS_X(param, _HAND_LOWER_LIMIT_UP)
	
#define HAS_LOWER_LIMITE_DOWN(param) \
	HAS_HAND_QOS_X(param, _HAND_LOWER_LIMIT_DOWN)
	
#define SET_LOWER_LIMITE_UP(param) \
	SET_HAND_QOS_X(param, _HAND_LOWER_LIMIT_UP)
	
#define SET_LOWER_LIMITE_DOWN(param) \
	SET_HAND_QOS_X(param, _HAND_LOWER_LIMIT_DOWN)

typedef struct hand_qos_max_uplimit_s{
	uint32_t mask;
	int up_val;
	int down_val;
	float min_up_val;
	float min_down_val;
}hand_qos_max_uplimit_t;

enum {
	_HAND_QOS_ACK_INFO_ = 0,
	_HAND_QOS_ACK_GLOBAL_EN,
	_HAND_QOS_ACK_MAX_UPLIMIT,
};
#define SET_HAND_QOS_ACK_INFO(ack)		\
	SET_HAND_QOS_X(ack,_HAND_QOS_ACK_INFO_)
#define SET_HAND_QOS_ACK_GLOBAL_EN(ack)		\
	SET_HAND_QOS_X(ack,_HAND_QOS_ACK_GLOBAL_EN)
#define SET_HAND_QOS_ACK_MAX_UPLIMIT(ack)		\
	SET_HAND_QOS_X(ack,_HAND_QOS_ACK_MAX_UPLIMIT)
	
#define HAS_HAND_QOS_ACK_INFO(ack) \
	HAS_HAND_QOS_X(ack,_HAND_QOS_ACK_INFO_)
#define HAS_HAND_QOS_ACK_GLOBAL_EN(ack) \
	HAS_HAND_QOS_X(ack,_HAND_QOS_ACK_GLOBAL_EN)
#define HAS_HAND_QOS_ACK_MAX_UPLIMIT(ack) \
	HAS_HAND_QOS_X(ack,_HAND_QOS_ACK_MAX_UPLIMIT)
	
typedef struct hand_qos_common_ack_s {
	int mask;
	int err_code;
	hand_qos_global_en_t g_enable;
	hand_qos_info_t info;
	hand_qos_max_uplimit_t max_uplimit;
}hand_qos_common_ack_t;
#endif

