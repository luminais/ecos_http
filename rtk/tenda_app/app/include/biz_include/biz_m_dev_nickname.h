#ifndef BIZ_M_DEV_NICKNAME_H
#define BIZ_M_DEV_NICKNAME_H

#include "process_api.h"

int biz_m_dev_nickname_info_get(
	const api_cmd_t *cmd,
 	dev_name_t *param,
	nickname_info_t *info,
	void *privdata);

int biz_m_dev_nickname_info_set(
	const api_cmd_t *cmd, 
	nickname_info_t *set_info,
	void *privdata);

#endif