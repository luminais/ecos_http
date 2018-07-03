#ifndef __M_OL_HOST_TYPES_H__
#define __M_OL_HOST_TYPES_H__
/*
**  Copyright (C), 1998-2014, Tenda Tech. Co., Ltd.
**
**  Project:	Cloud manager Api v1.0
**  File:    	
**  Author: 	Young
**  Date:    	07/08/2015
**
**  Purpose:
**    		.
**
**  History: 
**  <author>   <time>          <version >   <desc>
*	$Id: uc_lib_m_ol_host.h 241 2015-07-08 02:51:12Z yangyabing $
*/

#define OL_HOSTS_MAC_STR_LEN				20
#define OL_HOSTS_IP_STR_LEN					16
#define OL_HOSTS_NAME_STR_LEN				64
#define OL_HOSTS_SSID_STR_LEN				64
#define VENDOR_LEN							32

#define SET_OL_HOST_X(hosts,x)	\
	((hosts)->mask |= (1 << x))
#if 1
#define HAS_OL_HOST_X(hosts,x)\
	(((hosts)->mask & (1 << x)) == (1 << x))

#endif
typedef enum {
	OL_HOSTS_MAC_BLOCK = 0,
	OL_HOSTS_BW_LIMIT,
	OL_HOSTS_ONLINE_TIME,
	OL_HOSTS_UNDER_PC_CTRL,
	OL_HOSTS_UP_LIMIT,
	OL_HOSTS_DOWN_LIMIT,
	OL_HOSTS_DEV_TYPE,
	OL_HOSTS_DEV_TRUST,
	OL_HOSTS_DEV_ALIAS,
	OL_HOSTS_DEV_RELAT_SSID,
	OL_HOSTS_DEV_MANUFACTORY,
	OL_HOSTS_DEV_ONLINE_FLAG,
	OL_HOSTS_DEV_MANUFACTORY_DESC,
}__ol_hosts_get_dev_param;

#define SET_OL_HOSTS_MAC_BLOCK(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_MAC_BLOCK)
#define SET_OL_HOSTS_BW_LIMIT(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_BW_LIMIT)
#define SET_OL_HOSTS_ONLINE_TIME(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_ONLINE_TIME)	
#define SET_OL_HOSTS_UNDER_PC_CTRL(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_UNDER_PC_CTRL)
#define SET_OL_HOSTS_UP_LIMIT(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_UP_LIMIT)
#define SET_OL_HOSTS_DOWN_LIMIT(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DOWN_LIMIT)
#define SET_OL_HOSTS_DEV_TYPE(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_TYPE)
#define SET_OL_HOSTS_DEV_TRUST(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_TRUST)
#define SET_OL_HOSTS_DEV_ALIAS(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_ALIAS)
#define SET_OL_HOSTS_DEV_RELAT_SSID(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_RELAT_SSID)
#define SET_OL_HOSTS_DEV_MANUFACTORY(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_MANUFACTORY)
#define SET_OL_HOSTS_DEV_OL(ol_hosts_dev_t) \
	SET_OL_HOST_X(ol_hosts_dev_t, OL_HOSTS_DEV_ONLINE_FLAG)
#define SET_OL_HOSTS_DEV_MANUFACTORY_DESC(ol_hosts_dev_t)	\
	SET_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_MANUFACTORY_DESC)

#if 1
#define HAS_OL_HOSTS_MAC_BLOCK(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t, OL_HOSTS_MAC_BLOCK)
#define HAS_OL_HOSTS_BW_LIMIT(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_BW_LIMIT)
#define HAS_OL_HOSTS_ONLINE_TIME(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_ONLINE_TIME)	
#define HAS_OL_HOSTS_UNDER_PC_CTRL(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_UNDER_PC_CTRL)
#define HAS_OL_HOSTS_UP_LIMIT(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_UP_LIMIT)
#define HAS_OL_HOSTS_DOWN_LIMIT(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DOWN_LIMIT)
#define HAS_OL_HOSTS_DEV_TYPE(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_TYPE)
#define HAS_OL_HOSTS_DEV_TRUST(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_TRUST)
#define HAS_OL_HOSTS_DEV_ALIAS(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_ALIAS)
#define HAS_OL_HOSTS_DEV_RELAT_SSID(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_RELAT_SSID)
#define HAS_OL_HOSTS_DEV_MANUFACTORY(ol_hosts_dev_t) \
	HAS_OL_HOST_X(ol_hosts_dev_t, OL_HOSTS_DEV_MANUFACTORY)	
#define HAS_OL_HOSTS_DEV_OL(ol_hosts_dev_t) \
	HAS_OL_HOST_X(ol_hosts_dev_t, OL_HOSTS_DEV_ONLINE_FLAG)	
#define HAS_OL_HOSTS_DEV_MANUFACTORY_DESC(ol_hosts_dev_t)	\
	HAS_OL_HOST_X(ol_hosts_dev_t,OL_HOSTS_DEV_MANUFACTORY_DESC)
#endif

typedef struct ol_hosts_dev_s {
	int		mask;					//标记以下的数据选项是否存在
	int mac_blocked;				//是否处于黑名单控制下
	int bw_limited;					//是否带宽控制限速下
	int online_time;				//主机的在线时间
	int under_pc_control;			//是否处于家长控制下	
	int curr_up_rate;		//当前主机上行速率
	int curr_down_rate;		//当前主机下行速率  单位:KB/S	
	float up_limit;			//上行限速	
	float down_limit;			//下行限速  单位:5.23KB/S
	int host_type;			//主机类型	(iphone,andorid等)
	int host_manufactory;	//主机厂商
	int trust;				//是否被信任
	int online;
	int	 access_type;			//设备接入类型	(有线无线)
	char host_name[OL_HOSTS_NAME_STR_LEN];	//在线主机原名
	char host_alias[OL_HOSTS_NAME_STR_LEN];//在线主机别名
	char ip[OL_HOSTS_IP_STR_LEN];	//在线主机ip地址
	char mac[OL_HOSTS_MAC_STR_LEN];	//在线主机mac地址
	char asso_ssid[OL_HOSTS_SSID_STR_LEN];	//在线主机关联的ssid名(对于无线设备)
	char manufactory_desc[VENDOR_LEN];	//设备厂商描述，如Huawei，未识别置为空字符串
}ol_hosts_dev_t;

typedef struct ol_hosts_info_s {
	int mem_len;								//该结构体的实际长度
	int hosts_count;							//主机个数
	ol_hosts_dev_t hosts[0];					//主机信息柔性数组
}ol_hosts_info_t;

//ol_host module common ack
enum {
	_OL_HOSTS_ACK_INFO_ = 0,
};

#define HAS_OL_HOSTS_ACK_INFO(ack)	\
	HAS_OL_HOST_X(ack, _OL_HOSTS_ACK_INFO_)
#define SET_OL_HOSTS_ACK_INFO(ack)	\
	SET_OL_HOST_X(ack,_OL_HOSTS_ACK_INFO_)
	
typedef struct ol_host_common_ack_s {
	int mask;
	int err_code;
	ol_hosts_info_t info;
}ol_host_common_ack_t;
#endif

