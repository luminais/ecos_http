#ifndef __UC_M_SYS_TYPES_H__
#define __UC_M_SYS_TYPES_H__

#include <stdint.h>

#define MAX_TIME_ZONE_LENGTH 16

#define SET_SYS_X(sys, x)	\
	((sys)->mask |= (1 << x))
#define HAS_SYS_X(sys, x)	\
	(((sys)->mask & (1 << x)) == (1 << x))

enum {
	_SUPT_WAN_TYPE = 0,
};

#define SET_SUPT_WAN_TYPE(supt) \
	SET_SYS_X(supt, _SUPT_WAN_TYPE)
#define HAS_SUPT_WAN_TYPE(supt) \
	HAS_SYS_X(supt, _SUPT_WAN_TYPE)
	
enum {
	_SYS_BASIC_INFO_T = 0,
	_SYS_ADVANCE_INFO_T,
};

typedef enum{
	MODE_NONE=0,
	MODE_WDS,
	MODE_APCLIENT,
	MODE_WISP
}wl_mode_t;

typedef enum{
	DEV_MODE_UNSPEC = 0,
	DEV_MODE_AP,
	DEV_MODE_ROUTER,
}dev_mode_t;


typedef struct product_info_s {
	char firm[64];	//公司名Tenda
	char model[64];//产品名FH1901
	char soft_ver[32];//软件版本
	char hard_ver[32];//硬件版本
	uint32_t release_date; /* Since epoch */
} product_info_t;

typedef struct init_setup_info_s {
	uint32_t guide_done;//是否进行过设置向导0-未设置,1-已设置
} init_setup_info_t;

enum {
	_SUPT_MOD_CMD1 = 0,
	_SUPT_MOD_CMD2,
};

typedef struct sys_basic_info_s {
	int mask;
	char sn[32];//云服务获取的唯一序列号
	product_info_t product;
	init_setup_info_t init;
	char mac[20];
	wl_mode_t  wl_mode; //当前无线模式
	char supt_wan_type[128]; //wan支持的接入类型:例 "static,dhcp,adsl"
	dev_mode_t dev_mode;	//router work mode:ap/router/unspec/...
} sys_basic_info_t;

typedef struct cpu_info_s {
	uint32_t curr_idle;//current cpu idle example 80 percent
	uint32_t max_freq;// max cpu frequency example 300MHz
} cpu_info_t;

typedef struct mem_info_s {
	uint32_t used;//memory used example 365423KB
	uint32_t total;
} mem_info_t;

typedef struct sys_advance_info_s {
	cpu_info_t cpu_info;
	mem_info_t mem_info;
	uint32_t uptime;
	uint32_t systime; /* Since epoch */
} sys_advance_info_t;

typedef struct sys_time_zone_s {
	char timezone[MAX_TIME_ZONE_LENGTH];
}sys_time_zone_t;

#define SET_ACK_SYS_BASIC_INFO(ack) \
	SET_SYS_X(ack, _SYS_BASIC_INFO_T)
#define SET_ACK_ADVANCE_INFO(ack) \
	SET_SYS_X(ack, _SYS_ADVANCE_INFO_T)
#define HAS_ACK_SYS_BASIC_INFO(ack) \
	HAS_SYS_X(ack, _SYS_BASIC_INFO_T)
#define HAS_ACK_ADVANCE_INFO(ack) \
	HAS_SYS_X(ack, _SYS_ADVANCE_INFO_T)
	
typedef struct sys_common_ack_s {
	int mask;
	int err_code;
	sys_basic_info_t  basic;
	sys_advance_info_t advance;
}sys_common_ack_t;
#endif
