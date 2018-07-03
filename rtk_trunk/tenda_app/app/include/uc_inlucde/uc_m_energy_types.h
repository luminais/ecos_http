#ifndef __UC_M_ENERGY_TYPES_H__
#define __UC_M_ENERGY_TYPES_H__

//common mask set and has
#define SET_ENERGY_X(energy, x)	\
	((energy)->mask |= (1 << x))
#define HAS_ENERGY_X(energy, x)	\
	(((energy)->mask & (1 << x)) == (1 << x))
	
typedef struct _m_energy_led_t {
	int		led_status;
} m_energy_led_t;

typedef struct _m_energy_wireless_timer_t {
	int		timer_status;
	int		start_time;
	int		end_time;
	int		day;
} m_energy_wireless_timer_t;

typedef struct _m_energy_mode_t {
	int		mode_status;
} m_energy_mode_t;

//common ack
enum {
	_ENERGY_ACK_LED = 0,
	_ENERGY_ACK_WIRELESS_TIMER,
	_ENERGY_ACK_MODE,
};
#define SET_ENERGY_ACK_LED(energy)	\
	SET_ENERGY_X(energy, _ENERGY_ACK_LED)
#define SET_ENERGY_ACK_WIRELESS_TIMER(energy)	\
	SET_ENERGY_X(energy, _ENERGY_ACK_WIRELESS_TIMER)
#define SET_ENERGY_ACK_MODE(energy)	\
	SET_ENERGY_X(energy, _ENERGY_ACK_MODE)
#define HAS_ENERGY_ACK_LED(energy)	\
	HAS_ENERGY_X(energy, _ENERGY_ACK_LED)
#define HAS_ENERGY_ACK_WIRELESS_TIMER(energy)	\
	HAS_ENERGY_X(energy, _ENERGY_ACK_WIRELESS_TIMER)
#define HAS_ENERGY_ACK_MODE(energy)	\
	HAS_ENERGY_X(energy, _ENERGY_ACK_MODE)
	
typedef struct energy_common_ack_s {
	int mask;
	int err_code;
	m_energy_led_t led;
	m_energy_wireless_timer_t timer;
	m_energy_mode_t mode;
}energy_common_ack_t;

#endif