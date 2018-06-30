#ifndef BIZ_M_SAFE_CHECK_H
#define BIZ_M_SAFE_CHECK_H

#include "process_api.h"

int biz_m_safe_check_get_cb(
	const api_cmd_t *cmd, 
	safe_check_info_t *info, void *privdata);

int biz_m_dns_optimize(
	const api_cmd_t *cmd, 
	void *privdata);


#endif

