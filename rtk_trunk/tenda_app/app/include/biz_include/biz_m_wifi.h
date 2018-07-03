#ifndef BIZ_M_WIFI_H
#define BIZ_M_WIFI_H

int biz_m_wifi_power_get_cb(
	const api_cmd_t *cmd, 
	wifi_power_t *power,
	void *privdata);

int biz_m_wifi_power_set_cb(
	const api_cmd_t *cmd, 
	wifi_power_t *power,
	void *privdata);

int biz_m_wifi_basic_info_get_cb(
	const api_cmd_t *cmd, 
	wifi_basic_t *basic_info,
	void *privdata);

int biz_m_wifi_basic_info_set_cb(
	const api_cmd_t *cmd, 
	wifi_basic_t *basic_info,
	void *privdata);
#ifdef __CONFIG_GUEST__
int biz_m_wifi_guest_info_get_cb(
	const api_cmd_t *cmd, 
	wifi_basic_t *basic_info,
	void *privdata);

int biz_m_wifi_guest_info_set_cb(
	const api_cmd_t *cmd, 
	wifi_basic_t *basic_info,
	void *privdata);
#endif
int biz_m_wifi_channel_sta_get_cb(
	const api_cmd_t *cmd, 
	wifi_channel_info_t *chan,
	void *privdata);

int biz_m_wifi_channel_sta_set_cb(
	const api_cmd_t *cmd,
	void *privdata);

int biz_m_wifi_push_wifi_info_get(
	wifi_basic_t *info);
#ifdef CONFIG_APP_COSTDOWN
void biz_m_wifi_basic_info_set_process(void);
void biz_m_wifi_channel_sta_set_process(void);
void biz_m_wifi_power_set_process(void);

#ifdef __CONFIG_GUEST__
void biz_m_wifi_guest_info_set_process(void);
#endif

#endif
#endif
