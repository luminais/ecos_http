#ifndef BIZ_M_LOGIN_H
#define BIZ_M_LOGIN_H

#include "process_api.h"

int biz_m_login_login_cb(
	const api_cmd_t *cmd, 
 	m_login_t *login_info,
 	void *privdata);

#ifdef CONFIG_APP_COSTDOWN
int biz_m_login_pwd_sta_get_cb(
	const api_cmd_t *cmd,
	pwd_sta_t *sta,
	void *privdata);
#else
int biz_m_login_pwd_sta_get_cb(
	const api_cmd_t *cmd,
 	login_common_ack_t **ack_info,
 	void *privdata);
#endif

int biz_m_login_update_pwd_cb(
	const api_cmd_t *cmd, 
	m_login_update_pwd_t *update_info,
	void *privdata);

#endif