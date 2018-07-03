#include "biz_typedef.h"
#include "process_api.h"

/*
#include "biz_m_sys.h"
#include "biz_m_energy.h"
#include "biz_m_hand_qos.h"
#include "biz_m_login.h"
#include "biz_m_ol_host.h"
#include "biz_m_safe_check.h"
#include "biz_m_wan.h"
#include "biz_m_wifi.h"
#include "biz_m_dev_nickname.h"
#include "biz_m_rub_net.h"
#include "biz_m_upgrade.h"
#include "biz_m_ucloud_info.h"
*/

extern int biz_m_sys_reset_cb(const api_cmd_t *cmd, void *privdata);
extern int biz_m_sys_reboot_cb(const api_cmd_t *cmd, void *privdata);
extern int biz_m_sys_basic_info_get_cb(const api_cmd_t *cmd, sys_basic_info_t *basic,void *privdata);
extern int biz_m_sys_advance_info_get_cb(const api_cmd_t *cmd, sys_advance_info_t *advance,void *privdata);

extern int biz_wl_timer_get_cb(const api_cmd_t *cmd, m_energy_wireless_timer_t *timer_info,void *privdata);
extern int biz_wl_timer_set_cb(const api_cmd_t *cmd, m_energy_wireless_timer_t *timer_info,void *privdata);

extern int biz_hand_qos_info_get_cb (const api_cmd_t *cmd,hand_qos_get_param_t *param, hand_qos_common_ack_t **rules_info,void *privdata);
extern int biz_hand_qos_info_set_cb (const api_cmd_t *cmd, const hand_qos_set_info_t *set_info,void *privdata);
extern int biz_m_hand_qos_max_uplimit_cb(const api_cmd_t *cmd,hand_qos_max_uplimit_t *info,void *privdata);

extern int biz_m_login_login_cb(const api_cmd_t *cmd, m_login_t *login_info,void *privdata);
extern int biz_m_login_pwd_sta_get_cb(const api_cmd_t *cmd,login_common_ack_t **ack_info,void *privdata);
extern int biz_m_login_update_pwd_cb(const api_cmd_t *cmd, m_login_update_pwd_t *update_info,void *privdata);

extern int biz_m_ol_host_info_get_cb(const api_cmd_t *cmd, ol_host_common_ack_t ** hosts_info,void *privdata);

extern int biz_m_safe_check_get_cb(const api_cmd_t *cmd, safe_check_info_t *info, void *privdata);
extern int biz_m_dns_optimize(const api_cmd_t *cmd, void *privdata);

extern int biz_m_wan_basic_get_cb(const api_cmd_t *cmd, wan_basic_info_t *basic_info,void *privdata);
extern int biz_m_wan_basic_set_cb(const api_cmd_t *cmd, wan_basic_info_t *basic_info,void *privdata);
extern int biz_m_wan_rate_get_cb(const api_cmd_t *cmd,wan_rate_info_t *rate_info,void *privdata);
extern int biz_m_wan_detect_type_cb (const api_cmd_t *cmd, wan_detecttype_info_t *type_info,void *privdata);

extern int biz_m_wifi_power_get_cb(const api_cmd_t *cmd, wifi_power_t *power,void *privdata);
extern int biz_m_wifi_power_set_cb(const api_cmd_t *cmd, wifi_power_t *power,void *privdata);
extern int biz_m_wifi_basic_info_get_cb(const api_cmd_t *cmd, wifi_basic_t *basic_info,void *privdata);
extern int biz_m_wifi_basic_info_set_cb(const api_cmd_t *cmd, wifi_basic_t *basic_info,void *privdata);
extern int biz_m_wifi_channel_sta_get_cb(const api_cmd_t *cmd, wifi_channel_info_t *chan,void *privdata);
extern int biz_m_wifi_channel_sta_set_cb(const api_cmd_t *cmd,void *privdata);
extern int biz_m_wifi_push_wifi_info_get(wifi_basic_t *info);

extern int biz_m_dev_nickname_info_get(const api_cmd_t *cmd,dev_name_t *param,nickname_info_t *info,void *privdata);
extern int biz_m_dev_nickname_info_set(const api_cmd_t *cmd, nickname_info_t *set_info,void *privdata);

extern int biz_m_access_user_set_cb(const api_cmd_t *cmd,access_user_t *user_info,void *privdata);
extern int biz_m_rub_net_black_list_get_cb(const api_cmd_t *cmd,black_list_t **black_list, void *privdata);
extern int biz_m_strange_host_info_get(rub_strange_host_info_t **rub_info,int *n_host);
extern int biz_m_rub_net_mf_mode_get_cb(mac_filter_mode_t * mode,void * privdata);
extern int biz_m_rub_net_mf_mode_set_cb(mac_filter_mode_t * mode,void * privdata);
extern int biz_m_rub_net_black_list_flush_cb(void * privdata);


extern int biz_m_ucloud_info_download_path_get (struct download_path_t * dpath);
extern int biz_m_ucloud_info_memory_check_cb (struct mem_state_t *memory_state,int img_size);

extern int biz_m_ucloud_begin_upgrade_cb (api_cmd_t *cmd, unsigned int data_len, void *data,void *privdata);

extern int biz_m_ucloud_info_sn_set_cb(api_cmd_t *cmd, unsigned int data_len,void *data,void *privdata);
extern int biz_m_ucloud_info_basic_get(cloud_basic_info_t *data);
extern int biz_m_ucloud_info_manage_en_get_cb(const api_cmd_t *cmd,m_cloud_info_manage_en_t *manage_en,void *privdata);
extern int biz_m_ucloud_info_manage_en_set_cb(const api_cmd_t *cmd,m_cloud_info_manage_en_t *manage_en,void *privdata);
extern int biz_ucloud_info_try_clear_acc();
extern int biz_cloud_info_clear_account_ack_cb(const api_cmd_t *cmd, void *privdata);
#ifdef __CONFIG_GUEST__
extern int biz_m_wifi_guest_info_get_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
extern int biz_m_wifi_guest_info_set_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
#endif


int biz_m_sys_init(int fd)
{
	//BIZ_DEBUG("biz_m_sys_init\n");

	m_sys_regist_t  sys_info;
	memset(&sys_info, 0, sizeof(m_sys_regist_t));
		
	sys_info.m_sys_basic_info_get_cb = biz_m_sys_basic_info_get_cb;
	sys_info.basic_info_privdata = NULL;
	sys_info.m_sys_advance_info_get_cb = biz_m_sys_advance_info_get_cb;
	sys_info.advance_info_privdata = NULL;
	sys_info.m_sys_reset_cb = biz_m_sys_reset_cb;
	sys_info.reset_privdata = NULL;
	sys_info.m_sys_reboot_cb = biz_m_sys_reboot_cb;
	sys_info.reboot_privdata = NULL;
	sys_info.m_sys_set_time_zone_cb = NULL;
	sys_info.time_zone_privdata = NULL;
	
	if (-1 == uc_api_enable_m_sys(&sys_info)) {			  
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	return BIZ_RET_SUCCESS;
}

int biz_m_dev_nickname_init(int fd)
 {
	m_dev_nickname_regist_t dev_info = {NULL};
	//BIZ_DEBUG("biz_m_dev_nickname_init\n");

	dev_info.m_dev_nickname_get_cb = biz_m_dev_nickname_info_get;
	dev_info.m_dev_nickname_get_privdata = NULL;

	dev_info.m_dev_nickname_set_cb = biz_m_dev_nickname_info_set;
	dev_info.m_dev_nickname_set_privdata = NULL;
	
	if (-1 == uc_api_enable_m_dev_nickname(&dev_info)) {
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	
	return BIZ_RET_SUCCESS;
}

int biz_m_energy_init(int fd)
 {
	m_energy_regist_t energy_info = {NULL};
	//printf("biz_m_energy_init\n");
	energy_info.m_energy_mode_get_cb = NULL;
	energy_info.m_energy_mode_get_privdata = NULL;
	energy_info.m_energy_mode_set_cb = NULL;
	energy_info.m_energy_mode_set_privdata = NULL;
	
	energy_info.m_led_get_cb = NULL;
	energy_info.m_led_get_privdata = NULL;
	energy_info.m_led_set_cb = NULL;
	energy_info.m_led_set_privdata= NULL;

	energy_info.m_wl_timer_get_cb = biz_wl_timer_get_cb;
	energy_info.m_wl_timer_get_privdata= NULL;
	energy_info.m_wl_timer_set_cb= biz_wl_timer_set_cb;
	energy_info.m_wl_timer_set_privdata= NULL;

	if (-1 == uc_api_enable_m_energy(&energy_info)) {
		return -1;
	}

	uc_api_lib_commit(fd);
	return 0;
}

int biz_m_hand_qos_init(int fd)
 {
	//BIZ_DEBUG("biz_m_hand_qos_init\n");
	m_hand_qos_regist_t hand_qos_info = {NULL};

	hand_qos_info.m_hand_qos_get_cb = biz_hand_qos_info_get_cb;
	hand_qos_info.m_hand_qos_get_privdata = NULL;

	hand_qos_info.m_hand_qos_set_cb = biz_hand_qos_info_set_cb;
	hand_qos_info.m_hand_qos_set_privdata = NULL;

	hand_qos_info.m_hand_qos_get_genable_cb = NULL;
	hand_qos_info.m_hand_qos_get_genable_privdata = NULL;
	
	hand_qos_info.m_hand_qos_set_genable_cb = NULL;
	hand_qos_info.m_hand_qos_set_genable_privdata = NULL;

	hand_qos_info.m_hand_qos_max_uplimit_cb = biz_m_hand_qos_max_uplimit_cb;
	hand_qos_info.m_hand_qos_max_uplimit_privdata = NULL;
	
	if (-1 == uc_api_enable_m_hand_qos(&hand_qos_info)) {
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	
	return BIZ_RET_SUCCESS;
}

int biz_m_login_init(int fd)
 {
	m_login_regist_t login = {NULL};
	//BIZ_DEBUG("biz_m_login_init\n");
	login.m_login_login_info_cb = biz_m_login_login_cb;
	login.login_info_privdata = NULL;
	login.m_login_update_info_cb = biz_m_login_update_pwd_cb;
	login.update_info_privdata = NULL;
	login.m_login_get_pwd_sta_cb = biz_m_login_pwd_sta_get_cb;
	login.get_pwd_sta_privdata = NULL;

	if (-1 == uc_api_enable_m_login(&login)) {
		return BIZ_RET_FAILURE;
	}

	uc_api_lib_commit(fd);
	return BIZ_RET_SUCCESS;
}

int biz_m_ol_host_init(int fd)
 {

	//BIZ_DEBUG("biz_m_ol_host_init\n");
	
	m_ol_host_regist_t ol_host_info = {NULL};
	ol_host_info.m_ol_host_get_cb = biz_m_ol_host_info_get_cb;
	ol_host_info.m_ol_host_get_privdata = NULL;

	if (-1 == uc_api_enable_m_ol_host(&ol_host_info)) {
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	return BIZ_RET_SUCCESS;
}

int biz_m_rub_net_init(int fd)
{
	//BIZ_DEBUG("biz_m_rub_net_init\n");

	m_rub_net_regist_t net = {
		.m_access_user_set = biz_m_access_user_set_cb,
		.m_access_user_set_privdata = NULL,
		.m_rub_net_get = NULL,
		.m_rub_net_get_privdata = NULL,
		.m_rub_net_set = NULL,
		.m_rub_net_set_privdata = NULL,
		.m_history_list_get = NULL,
		.m_history_list_get_privdata = NULL,
		.m_history_list_clear = NULL,
		.m_history_list_get_privdata = NULL,
		.m_black_list_get = biz_m_rub_net_black_list_get_cb,
		.m_black_list_get_privdata = NULL,
		.m_macfilter_mode_get = biz_m_rub_net_mf_mode_get_cb,
		.m_macfilter_mode_get_privdata = NULL,
		.m_macfilter_mode_set = biz_m_rub_net_mf_mode_set_cb,
		.m_macfilter_mode_set_privdata = NULL,
		.m_black_list_flush = biz_m_rub_net_black_list_flush_cb,
		.m_black_list_flush_privdata = NULL,
		
	};
	if (-1 == uc_api_enable_m_rub_net(&net)){
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	
	return BIZ_RET_SUCCESS; 
}

int biz_m_safe_check_init(int fd)
{
	//BIZ_DEBUG("biz_m_safe_check_init\n");

	m_safe_check_regist_t safe = {
		.m_safe_check_get = biz_m_safe_check_get_cb,
		.m_safe_check_get_privdata = NULL,
		.m_dns_opt_set = biz_m_dns_optimize,
		.m_dns_opt_set_privdata = NULL,
	};
	if (-1 == uc_api_enable_m_safe_check(&safe)){
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	return BIZ_RET_SUCCESS; 
}

int biz_m_wan_init(int fd)
 {
	m_wan_regist_t wan_info = {NULL};
	//BIZ_DEBUG("biz_m_wan_init\n");
	wan_info.m_wan_rate_get_cb  = biz_m_wan_rate_get_cb;
	wan_info.m_wan_rate_get_privdata = NULL;
	wan_info.m_wan_basic_set_cb = biz_m_wan_basic_set_cb;
	wan_info.m_wan_basic_set_privdata = NULL;
	wan_info.m_wan_basic_get_cb = biz_m_wan_basic_get_cb;
	wan_info.m_wan_basic_get_privdata= NULL;
	wan_info.m_wan_detect_type_cb = biz_m_wan_detect_type_cb;
	wan_info.m_wan_detect_type_privdata = NULL;

	if (-1 == uc_api_enable_m_wan(&wan_info)) {
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	return BIZ_RET_SUCCESS;
}

int biz_m_wifi_init(int fd)
 {
	m_wifi_regist_t wifi_info = {NULL};
	//BIZ_DEBUG("biz_m_wifi_init\n");

	wifi_info.m_wifi_basic_info_get_cb = biz_m_wifi_basic_info_get_cb;
	wifi_info.wifi_basic_info_get_privdata = NULL;
	wifi_info.m_wifi_basic_info_set_cb = biz_m_wifi_basic_info_set_cb;
	wifi_info.wifi_basic_info_set_privdata = NULL;

	wifi_info.m_wifi_channel_get_cb = biz_m_wifi_channel_sta_get_cb;
	wifi_info.wifi_channel_get_privdata = NULL;
	wifi_info.m_wifi_channel_set_cb = biz_m_wifi_channel_sta_set_cb;
	wifi_info.wifi_channel_set_privdata = NULL;
	
#ifdef __CONFIG_GUEST__
	wifi_info.m_wifi_guest_info_get_cb = biz_m_wifi_guest_info_get_cb;
	wifi_info.wifi_guest_info_get_privdata = NULL;
	wifi_info.m_wifi_guest_info_set_cb = biz_m_wifi_guest_info_set_cb;
	wifi_info.wifi_guest_info_set_privdata = NULL;
#endif

	wifi_info.m_wifi_power_get_cb = biz_m_wifi_power_get_cb;
	wifi_info.wifi_power_get_privdata = NULL;
	wifi_info.m_wifi_power_set_cb = biz_m_wifi_power_set_cb;
	wifi_info.wifi_power_set_privdata = NULL;
	
	if (-1 == uc_api_enable_m_wifi(&wifi_info)) {
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);
	
	return BIZ_RET_SUCCESS;
}

static int biz_set_basic_info()
{
	cloud_basic_info_t data = {0};
	biz_m_ucloud_info_basic_get(&data);
	
	data.sn_set_cb = biz_m_ucloud_info_sn_set_cb;
	data.sn_set_priv = NULL;
	
	if (-1 == uc_api_set_basic_info(&data)) {
		return BIZ_RET_FAILURE;
	}
	return BIZ_RET_SUCCESS;
}

//ucloud info
static int biz_enable_upgrade() 
{
	upgrade_info_t up_info = {{0},0};
	
	struct download_path_t dpath;
	biz_m_ucloud_info_download_path_get(&dpath);
	strncpy(&up_info.upgrade_dir[0], dpath.path, dpath.len);
	up_info.upgrade_dir[dpath.len] = '\0';
	free(dpath.path);
	
	up_info.memory_check_cb = biz_m_ucloud_info_memory_check_cb;
	up_info.memory_check_priv = NULL;
	up_info.begin_upgrade_cb = biz_m_ucloud_begin_upgrade_cb;
	up_info.begin_upgrade_priv = NULL;
	if (-1 == uc_api_enable_upgrade(&up_info)) {
		return BIZ_RET_FAILURE;
	}
	return BIZ_RET_SUCCESS;
}

static int biz_m_up_speed_cb(
	api_cmd_t *cmd, unsigned int data_len, 
	void *data, void *privdata)
{
	struct speed *up = (struct speed *)data;
	return BIZ_RET_SUCCESS;
}

static int biz_enable_test_bandwidth() 
{
	speed_test_info_t speed_info = {NULL};
	speed_info.up_speed_set_cb = biz_m_up_speed_cb;
	speed_info.up_speed_set_priv = NULL;
	if (-1 == uc_api_enable_speed_test(&speed_info)) {
		return BIZ_RET_FAILURE;
	}
	return BIZ_RET_SUCCESS;
}

static int biz_enable_cloud_manage()
{
	m_cloud_info_regist_t info = {
		.cloud_info_manage_en_get_cb = biz_m_ucloud_info_manage_en_get_cb,
		.cloud_info_manage_en_get_privdata = NULL,
		.cloud_info_manage_en_set_cb = biz_m_ucloud_info_manage_en_set_cb,
		.cloud_info_manage_en_set_privdata = NULL,
		.cloud_info_clear_account_ack_cb = biz_cloud_info_clear_account_ack_cb,
		.cloud_info_clear_account_ack_privdata = NULL,
	};

	if (-1 == uc_api_enable_m_cloud_info(&info)) {
		return BIZ_RET_FAILURE;
	}
	return BIZ_RET_SUCCESS;
}

int biz_ucloud_info_init(int fd)
{

	if (-1 == biz_set_basic_info()){
		BIZ_ERROR("biz_set_basic_info failed\n");
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	if (BIZ_RET_FAILURE == biz_enable_upgrade()){
		BIZ_ERROR("biz_enable_upgrade failed\n");
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	if (BIZ_RET_FAILURE == biz_enable_test_bandwidth()){
		BIZ_ERROR("biz_enable_test_bandwidth failed\n");
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	if (BIZ_RET_FAILURE == biz_enable_cloud_manage()){
		BIZ_ERROR("biz_enable_cloud_manage failed\n");
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

	if (BIZ_RET_FAILURE == biz_ucloud_info_try_clear_acc()) {
		BIZ_ERROR("biz_ucloud_info_try_clear_acc failed\n");
		return BIZ_RET_FAILURE;
	}
	uc_api_lib_commit(fd);

    return BIZ_RET_SUCCESS; 
}

