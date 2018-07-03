#ifndef BIZ_M_HAND_QOS_H
#define BIZ_M_HAND_QOS_H

#include "process_api.h"







int biz_hand_qos_info_get_cb (
	const api_cmd_t *cmd,
 	hand_qos_get_param_t *param, 
	hand_qos_common_ack_t **rules_info,
	void *privdata);

int biz_hand_qos_info_set_cb (
	const api_cmd_t *cmd, 
	const hand_qos_set_info_t *set_info,
	void *privdata);

int biz_m_hand_qos_max_uplimit_cb(
	const api_cmd_t *cmd,
	hand_qos_max_uplimit_t *info,
	void *privdata);

#endif

