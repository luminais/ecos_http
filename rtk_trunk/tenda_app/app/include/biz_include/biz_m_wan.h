#ifndef BIZ_M_WAN_H
#define BIZ_M_WAN_H

#include "process_api.h"

enum{
	REMOTE_SERVER_NO_RESPONSE=548,       //远端服务器无响应
	USERNAME_PASSWD_ERROR=549,           //用户名或密码错误
	NOT_CONNECT_NETWORK=566,             //无法连接到互联网
};

enum{
	NOT_LINK= 0,                         //未插网线
	NOT_NETWORK,                         //未联网
	NETWORKING,                          //联网中
	NETWORKED,                           //已联网
};

int biz_m_wan_basic_get_cb(
	const api_cmd_t *cmd, 
	wan_basic_info_t *basic_info,
	void *privdata);

int biz_m_wan_basic_set_cb(
	const api_cmd_t *cmd, 
	wan_basic_info_t *basic_info,
	void *privdata);

int biz_m_wan_rate_get_cb(
	const api_cmd_t *cmd,
	wan_rate_info_t *rate_info,
	void *privdata);

int biz_m_wan_detect_type_cb (
	const api_cmd_t *cmd, 
	wan_detecttype_info_t *type_info,
	void *privdata);

#endif