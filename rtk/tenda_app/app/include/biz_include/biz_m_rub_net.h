#ifndef BIZ_M_RUB_NET_H
#define BIZ_M_RUB_NET_H

#include "process_api.h"






int biz_m_access_user_set_cb( 
	const api_cmd_t *cmd,
	access_user_t *user_info,
	void *privdata);

int biz_m_rub_net_black_list_get_cb(
		const api_cmd_t *cmd,
		black_list_t **black_list,
#ifdef CONFIG_APP_COSTDOWN
		int head_keep_size,
#endif
		void *privdata);

int biz_m_strange_host_info_get(
	rub_strange_host_info_t **rub_info,
	int *n_host);

int biz_m_rub_net_mf_mode_get_cb(
	mac_filter_mode_t * mode, 
	void * privdata);

int biz_m_rub_net_mf_mode_set_cb(
	mac_filter_mode_t * mode, 
	void * privdata);

int biz_m_rub_net_black_list_flush_cb(void * privdata);

#endif

