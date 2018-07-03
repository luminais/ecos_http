#ifndef __ATE_H__
#define __ATE_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef uint16 chanspec_t;
#define WLC_UP					2
#define WLC_DOWN				3
#define BCME_OK					0	/* Success */
#define WLC_GET_UP				162

typedef enum
{
	ATE_EXIT = 0,
	ATE_RUN = 1,
}ATESTATUS;

typedef enum
{
	SUPPORT_POWER_2G = 0,
	SUPPORT_POWER_5G = 1,
	SUPPORT_POWER_2G_5G = 2, 
	SUPPORT_POWER_MAX = 3,
}SUPPORT_POWER;

typedef enum
{
	MAX_POWER_2G = 0,
	MAX_POWER_5G = 1,
	MAX_POWER_2G_5G = 2, 
	MAX_POWER_MAX = 3,
}MAX_POWER;


#define SUCCESS  0
#define ERROR    -1
#define INFO     1

typedef struct
{
	uint8 ant_config[4];	/* antenna configuration */
	uint8 num_antcfg;	/* number of available antenna configurations */
} wlc_antselcfg_t;

extern void ate_start(void);
#endif/*__ATE_H__*/
