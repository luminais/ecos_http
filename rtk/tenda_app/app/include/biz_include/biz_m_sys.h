#ifndef BIZ_M_SYS_H
#define BIZ_M_SYS_H

#include "process_api.h"

int biz_m_sys_reset_cb(
	const api_cmd_t *cmd, 
	void *privdata);

int biz_m_sys_basic_info_get_cb(
	const api_cmd_t *cmd, 
	sys_basic_info_t *basic,
	void *privdata);

int biz_m_sys_advance_info_get_cb(
	const api_cmd_t *cmd, 
    sys_advance_info_t *advance,
    void *privdata);

#ifdef CONFIG_APP_COSTDOWN
	void biz_m_sys_reset(void);
	void biz_m_sys_reboot(void);
#else
int biz_m_sys_reboot_cb(
	const api_cmd_t *cmd,
	void *privdata);
#endif
#endif

