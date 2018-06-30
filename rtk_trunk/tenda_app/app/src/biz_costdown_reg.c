#include "biz_typedef.h"
#include "process_api.h"

extern int biz_m_sys_reset_cb(const api_cmd_t *cmd, void *privdata);
extern int biz_m_sys_reboot_cb(const api_cmd_t *cmd, void *privdata);
extern int biz_m_sys_basic_info_get_cb(const api_cmd_t *cmd, sys_basic_info_t *basic,void *privdata);
extern int biz_m_sys_advance_info_get_cb(const api_cmd_t *cmd, sys_advance_info_t *advance,void *privdata);

extern int biz_wl_timer_get_cb(const api_cmd_t *cmd, m_energy_wireless_timer_t *timer_info,void *privdata);
extern int biz_wl_timer_set_cb(const api_cmd_t *cmd, m_energy_wireless_timer_t *timer_info,void *privdata);
extern int biz_m_led_get_cb ( const api_cmd_t *cmd,
						  	   m_energy_led_t *led_info,
							   void *privdata);
extern int biz_m_led_set_cb (const api_cmd_t *cmd,
							  m_energy_led_t *led_info,
							  void *privdata);

extern int biz_hand_qos_info_get_cb (const api_cmd_t *cmd,hand_qos_get_param_t *param, hand_qos_common_ack_t **rules_info,void *privdata);
extern int biz_hand_qos_info_set_cb (const api_cmd_t *cmd, const hand_qos_set_info_t *set_info,void *privdata);
extern int biz_m_hand_qos_max_uplimit_cb(const api_cmd_t *cmd,hand_qos_max_uplimit_t *info,void *privdata);

extern int biz_m_login_login_cb(const api_cmd_t *cmd, m_login_t *login_info,void *privdata);
extern int biz_m_login_pwd_sta_get_cb(const api_cmd_t *cmd, pwd_sta_t *sta,void *privdata);
extern int biz_m_login_update_pwd_cb(const api_cmd_t *cmd, m_login_update_pwd_t *update_info,void *privdata);

extern int biz_m_ol_host_info_get_cb(const api_cmd_t *cmd, ol_host_common_ack_t ** hosts_info,void *privdata);

extern int biz_m_safe_check_get_cb(const api_cmd_t *cmd, safe_check_info_t *info, void *privdata);
extern int biz_m_dns_optimize(const api_cmd_t *cmd, void *privdata);

extern int biz_m_wan_basic_get_cb(const api_cmd_t *cmd, wan_basic_info_t *basic_info,void *privdata);
extern int biz_m_wan_basic_set_cb(const api_cmd_t *cmd, wan_basic_info_t *basic_info,void *privdata);
extern void biz_m_wan_basic_set_process(void);

extern int biz_m_wan_rate_get_cb(const api_cmd_t *cmd,wan_rate_info_t *rate_info,void *privdata);
extern int biz_m_wan_detect_type_cb (const api_cmd_t *cmd, wan_detecttype_info_t *type_info,void *privdata);

extern int biz_m_wifi_power_get_cb(const api_cmd_t *cmd, wifi_power_t *power,void *privdata);
extern int biz_m_wifi_power_set_cb(const api_cmd_t *cmd, wifi_power_t *power,void *privdata);
extern void biz_m_wifi_power_set_process(void);
extern int biz_m_wifi_basic_info_get_cb(const api_cmd_t *cmd, wifi_basic_t *basic_info,void *privdata);
extern int biz_m_wifi_basic_info_set_cb(const api_cmd_t *cmd, wifi_basic_t *basic_info,void *privdata);
extern void biz_m_wifi_basic_info_set_process(void);
extern int biz_m_wifi_channel_sta_get_cb(const api_cmd_t *cmd, wifi_channel_info_t *chan,void *privdata);
extern int biz_m_wifi_channel_sta_set_cb(const api_cmd_t *cmd,void *privdata);
extern void biz_m_wifi_channel_sta_set_process(void);
extern int biz_m_wifi_guest_info_get_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
extern int biz_m_wifi_guest_info_set_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
extern void biz_m_wifi_guest_info_set_process(void);

extern int biz_m_wifi_push_wifi_info_get(wifi_basic_t *info);

extern int biz_m_dev_nickname_info_get(const api_cmd_t *cmd,dev_name_t *param,nickname_info_t *info,void *privdata);
extern int biz_m_dev_nickname_info_set(const api_cmd_t *cmd, nickname_info_t *set_info,void *privdata);

extern int biz_m_access_user_set_cb(const api_cmd_t *cmd,access_user_t *user_info,void *privdata);
extern int biz_m_rub_net_black_list_get_cb(const api_cmd_t *cmd,black_list_t **black_list, void *privdata);
extern int biz_m_strange_host_info_get(rub_strange_host_info_t **rub_info,int *n_host);
extern int biz_m_rub_net_mf_mode_get_cb(mac_filter_mode_t * mode,void * privdata);
extern int biz_m_rub_net_mf_mode_set_cb(mac_filter_mode_t * mode,void * privdata);
extern int biz_m_rub_net_black_list_flush_cb(void * privdata);

#ifdef __CONFIG_GUEST__
extern int biz_m_wifi_guest_info_get_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
extern int biz_m_wifi_guest_info_set_cb (const api_cmd_t *cmd, guest_info_t *guest_info, void *privdata);
#endif

static m_sys_regist_t sys_info = {
	.m_sys_basic_info_get_cb = biz_m_sys_basic_info_get_cb,
	.m_sys_advance_info_get_cb = biz_m_sys_advance_info_get_cb,
	.m_sys_set_time_zone_cb = NULL,
};

inline basic_info_fn_t *biz_costd_getfunc_basici(void)
{
	return sys_info.m_sys_basic_info_get_cb;
}

inline advance_info_fn_t *biz_costd_getfunc_advancei(void)
{
	return sys_info.m_sys_advance_info_get_cb;
}

inline set_time_zone_fn_t *biz_costd_getfunc_sys_set_timez(void)
{
	return sys_info.m_sys_set_time_zone_cb;
}
 
static m_dev_nickname_regist_t dev_info = {
	.m_dev_nickname_get_cb = biz_m_dev_nickname_info_get,
	.m_dev_nickname_set_cb = biz_m_dev_nickname_info_set,
};

inline dev_nickname_get_fn_t *biz_costd_getfunc_m_d_nicknamei_get(void)
{
	return dev_info.m_dev_nickname_get_cb;
}

inline dev_nickname_set_fn_t *biz_costd_getfunc_m_d_nicknamei_set(void)
{
	return dev_info.m_dev_nickname_set_cb;
}

static m_energy_regist_t energy_info = {
	.m_energy_mode_get_cb = NULL,
	.m_energy_mode_set_cb = NULL,
	.m_led_get_cb = biz_m_led_get_cb,
	.m_led_set_cb = biz_m_led_set_cb,
	.m_wl_timer_get_cb = biz_wl_timer_get_cb,
	.m_wl_timer_set_cb= biz_wl_timer_set_cb,
};

inline energy_mode_get_fn_t *biz_costd_getfunc_m_energy_mode_get(void)
{
	return energy_info.m_energy_mode_get_cb;
}

inline energy_mode_set_fn_t *biz_costd_getfunc_m_energy_mode_set(void)
{
	return energy_info.m_energy_mode_set_cb;
}

inline led_get_fn_t  *biz_costd_getfunc_m_energy_led_get(void)
{
	return energy_info.m_led_get_cb;
}

inline led_set_fn_t *biz_costd_getfunc_m_energy_led_set(void)
{
	return energy_info.m_led_set_cb;
}

inline wl_timer_get_fn_t *biz_costd_getfunc_m_energy_wl_timer_get(void)
{
	return energy_info.m_wl_timer_get_cb;
}

inline wl_timer_set_fn_t *biz_costd_getfunc_m_energy_wl_timer_set(void)
{
	return energy_info.m_wl_timer_set_cb;
}

static m_hand_qos_regist_t hand_qos_info = {
	.m_hand_qos_get_cb = biz_hand_qos_info_get_cb,
	.m_hand_qos_set_cb = biz_hand_qos_info_set_cb,
	.m_hand_qos_get_genable_cb = NULL,
	.m_hand_qos_set_genable_cb = NULL,
	.m_hand_qos_max_uplimit_cb = biz_m_hand_qos_max_uplimit_cb,
};

inline hand_qos_get_fn_t *biz_costd_getfunc_hand_qos_get(void)
{
	return hand_qos_info.m_hand_qos_get_cb;
}

inline hand_qos_set_fn_t *biz_costd_getfunc_hand_qos_set(void)
{
	return hand_qos_info.m_hand_qos_set_cb;
}

inline hand_qos_get_genable_fn_t  *biz_costd_getfunc_m_hand_qos_get_genable(void)
{
	return hand_qos_info.m_hand_qos_get_genable_cb;
}

inline hand_qos_set_genable_fn_t *biz_costd_getfunc_m_hand_qos_set_genable(void)
{
	return hand_qos_info.m_hand_qos_set_genable_cb;
}

inline hand_qos_max_uplimit_fn_t *biz_costd_getfunc_m_hand_qos_max_uplimit(void)
{
	return hand_qos_info.m_hand_qos_max_uplimit_cb;
}

static m_login_regist_t login = {
	.m_login_login_info_cb = biz_m_login_login_cb,
	.m_login_update_info_cb = biz_m_login_update_pwd_cb,
	.m_login_get_pwd_sta_cb = biz_m_login_pwd_sta_get_cb,
};
inline login_fn_t  *biz_costd_getfunc_m_login_login_info(void)
{
	return login.m_login_login_info_cb;
}

inline up_pwd_fn_t *biz_costd_getfunc_m_login_update_info(void)
{
	return login.m_login_update_info_cb;
}

inline get_pwd_sta_fn_t *biz_costd_getfunc_m_login_get_pwd_sta(void)
{
	return login.m_login_get_pwd_sta_cb;
}

static m_ol_host_regist_t ol_host_info = {
	.m_ol_host_get_cb = biz_m_ol_host_info_get_cb,
};

inline ol_hosts_get_cb_fn_t *biz_costd_getfunc_m_ol_host_get_cb(void)
{
	return ol_host_info.m_ol_host_get_cb;
}

static m_rub_net_regist_t net = {
	.m_access_user_set = biz_m_access_user_set_cb,
	.m_rub_net_get = NULL,
	.m_rub_net_set = NULL,
	.m_history_list_get = NULL,
	.m_history_list_clear = NULL,
	.m_black_list_get = biz_m_rub_net_black_list_get_cb,
	.m_macfilter_mode_get = biz_m_rub_net_mf_mode_get_cb,
	.m_macfilter_mode_set = biz_m_rub_net_mf_mode_set_cb,
	.m_black_list_flush = biz_m_rub_net_black_list_flush_cb,	
};

inline access_user_fn_t  *biz_costd_getfunc_m_access_user_set(void)
{
	return net.m_access_user_set;
}

inline rub_net_get_fn_t *biz_costd_getfunc_m_rub_net_get(void)
{
	return net.m_rub_net_get;
}

inline rub_net_set_fn_t *biz_costd_getfunc_m_rub_net_set(void)
{
	return net.m_rub_net_set;
}

inline history_list_get_fn_t  *biz_costd_getfunc_m_history_list_get(void)
{
	return net.m_history_list_get;
}

inline history_list_clear_fn_t *biz_costd_getfunc_m_history_list_clear(void)
{
	return net.m_history_list_clear;
}

inline black_list_get_fn_t *biz_costd_getfunc_m_black_list_get(void)
{
	return net.m_black_list_get;
}

inline mf_mode_get_fn_t *biz_costd_getfunc_m_macfilter_mode_get(void)
{
	return net.m_macfilter_mode_get;
}

inline mf_mode_set_fn_t *biz_costd_getfunc_m_macfilter_mode_set(void)
{
	return net.m_macfilter_mode_set;
}

inline black_list_flush_fn_t *biz_costd_getfunc_m_black_list_flush(void)
{
	return net.m_black_list_flush;
}

static m_safe_check_regist_t safe = {
	.m_safe_check_get = biz_m_safe_check_get_cb,
	.m_dns_opt_set = biz_m_dns_optimize,
};

inline safe_check_fn_t *biz_costd_getfunc_m_safe_check_get(void)
{
	return safe.m_safe_check_get;
}

inline dns_optimize_fn_t *biz_costd_getfunc_m_dns_opt_set(void)
{
	return safe.m_dns_opt_set;
}

static m_wan_regist_t wan_info = {
	.m_wan_rate_get_cb  = biz_m_wan_rate_get_cb,
	.m_wan_basic_set_cb = biz_m_wan_basic_set_cb,
	.m_wan_basic_get_cb = biz_m_wan_basic_get_cb,
	.m_wan_detect_type_cb = biz_m_wan_detect_type_cb,
};

inline get_rate_cb_fn_t *biz_costd_getfunc_m_wan_rate_get(void)
{
	return wan_info.m_wan_rate_get_cb;
}

inline basic_set_cb_fn_t *biz_costd_getfunc_m_wan_basic_set(void)
{
	return wan_info.m_wan_basic_set_cb;
}
static timer_delay_cb *delay_wan_basic_func = biz_m_wan_basic_set_process;
inline timer_delay_cb *biz_costd_getfunc_m_wan_basic_timer_delay(void)
{
	return delay_wan_basic_func;
}

inline basic_get_cb_fn_t *biz_costd_getfunc_m_wan_basic_get(void)
{
	return wan_info.m_wan_basic_get_cb;
}

inline detect_type_cb_fn_t *biz_costd_getfunc_m_wan_detect_type(void)
{
	return wan_info.m_wan_detect_type_cb;
}

static m_wifi_regist_t wifi_info = {
	.m_wifi_basic_info_get_cb = biz_m_wifi_basic_info_get_cb,
	.m_wifi_basic_info_set_cb = biz_m_wifi_basic_info_set_cb,
	.m_wifi_channel_get_cb = biz_m_wifi_channel_sta_get_cb,
	.m_wifi_channel_set_cb = biz_m_wifi_channel_sta_set_cb,
#ifdef __CONFIG_GUEST__
	.m_wifi_guest_info_get_cb = biz_m_wifi_guest_info_get_cb,
	.m_wifi_guest_info_set_cb = biz_m_wifi_guest_info_set_cb,
#endif
#ifdef __CONFIG_APP_SIGNAL_STRENGTH_HIDE__
	.m_wifi_power_get_cb = NULL,
	.m_wifi_power_set_cb = NULL,
#else
	.m_wifi_power_get_cb = biz_m_wifi_power_get_cb,
	.m_wifi_power_set_cb = biz_m_wifi_power_set_cb,
#endif

};

inline wifi_basic_info_get_fn_t *biz_costd_getfunc_m_wifi_basic_info_get(void)
{
	return wifi_info.m_wifi_basic_info_get_cb;
}


inline wifi_basic_info_set_fn_t *biz_costd_getfunc_m_wifi_basic_info_set(void)
{
	return wifi_info.m_wifi_basic_info_set_cb;
}
static timer_delay_cb *delay_wifi_basic_func = biz_m_wifi_basic_info_set_process;

inline timer_delay_cb *biz_costd_getfunc_m_wifi_basic_timer_delay(void)
{
	return delay_wifi_basic_func;
}

inline wifi_guest_info_get_fn_t *biz_costd_getfunc_m_wifi_guest_info_get(void)
{
	return wifi_info.m_wifi_guest_info_get_cb;
}

inline wifi_guest_info_set_fn_t *biz_costd_getfunc_m_wifi_guest_info_set(void)
{
	return wifi_info.m_wifi_guest_info_set_cb;
}

static timer_delay_cb *delay_wifi_guest_func = biz_m_wifi_guest_info_set_process;
inline timer_delay_cb *biz_costd_getfunc_m_wifi_guest_timer_delay(void)
{
	return delay_wifi_guest_func;
}

inline wifi_channel_get_fn_t *biz_costd_getfunc_m_wifi_channel_get(void)
{
	return wifi_info.m_wifi_channel_get_cb;
}

inline wifi_channel_set_fn_t *biz_costd_getfunc_m_wifi_channel_set(void)
{
	return wifi_info.m_wifi_channel_set_cb;
}

static timer_delay_cb *delay_wifi_channel_func = biz_m_wifi_channel_sta_set_process;
inline timer_delay_cb *biz_costd_getfunc_m_wifi_channel_timer_delay(void)
{
	return delay_wifi_channel_func;
}

inline wifi_power_get_fn_t *biz_costd_getfunc_m_wifi_power_get(void)
{
	return wifi_info.m_wifi_power_get_cb;
}

inline wifi_power_set_fn_t *biz_costd_getfunc_m_wifi_power_set(void)
{
	return wifi_info.m_wifi_power_set_cb;
}

static timer_delay_cb *delay_wifi_power_func = biz_m_wifi_power_set_process;
inline timer_delay_cb *biz_costd_getfunc_m_wifi_power_timer_delay(void)
{
	return delay_wifi_power_func;
}

