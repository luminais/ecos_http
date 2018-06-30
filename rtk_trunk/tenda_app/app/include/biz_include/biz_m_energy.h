#ifndef BIZ_M_ENERGY_H
#define BIZ_M_ENERGY_H

#include "process_api.h"

int biz_wl_timer_get_cb(
	const api_cmd_t *cmd, 
    m_energy_wireless_timer_t *timer_info,
	void *privdata);

int biz_wl_timer_set_cb(
	const api_cmd_t *cmd, 
    m_energy_wireless_timer_t *timer_info,
	void *privdata);

int biz_m_led_get_cb ( 
	const api_cmd_t *cmd,
	m_energy_led_t *led_info,
	void *privdata);

int biz_m_led_set_cb ( 
	const api_cmd_t *cmd,
	m_energy_led_t *led_info,
	void *privdata);
#endif

