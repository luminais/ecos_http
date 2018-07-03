#ifndef BIZ_M_OL_HOST_H
#define BIZ_M_OL_HOST_H

#include "process_api.h"


int biz_m_ol_host_info_get_cb(
	const api_cmd_t *cmd, 
	ol_host_common_ack_t ** hosts_info,
	void *privdata);


typedef struct biz_m_rub_net_host_trust_detail_s {
	char mac[MAC_STRING_LEN];
	int is_trust;
}biz_m_rub_net_host_trust_detail_t;


typedef struct biz_m_rub_net_host_trust_info_s {
	int n_detail;
	biz_m_rub_net_host_trust_detail_t detail[0];
}biz_m_rub_net_host_trust_info_t;


#endif

