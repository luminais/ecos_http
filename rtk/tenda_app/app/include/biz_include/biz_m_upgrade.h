#ifndef BIZ_M_UPGRADE_H
#define BIZ_M_UPGRADE_H

#include "process_api.h"



int biz_m_ucloud_info_download_path_get (
	struct download_path_t * dpath);

int biz_m_ucloud_info_memory_check_cb (
	struct mem_state_t *memory_state,
	int img_size);

int biz_m_ucloud_begin_upgrade_cb (
    api_cmd_t *cmd, 
	unsigned int data_len, 
	void *data,
	void *privdata);


#endif

