#ifndef __WAN_MODE_CHECK_H_
#define __WAN_MODE_CHECK_H_

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

typedef enum check_result
{ 
	CHECK_ING = 0, 
	CHECK_END,
}CHECK_RESULT;

typedef enum check_type
{ 
	CHECK_DHCP_PING_PPPOE = 0, 
	CHECK_DHCP_PPPOE,
	CHECK_PPPOE_DHCP,
}CHECK_TYPE;

#define PPPOE_CHECK_MAX_NUM 	3
#define DHCP_CHECK_MAX_NUM  	3

typedef struct wan_mode_check_info_struct
{
	PIU8 enable;

	CHECK_TYPE type;

	PIU8 exit_tag;
	
	CHECK_RESULT check_finish_tag;

	WANMODE wan_mode;
	WANMODE check_result;

	PIU8 discover_send_num;
	PIU8 offer_rcv_tag;
	
	PIU8 padi_send_num;
	PIU8 pado_rcv_tag;	
}WAN_MODE_CHECK_INFO_STRUCT,*P_WAN_MODE_CHECK_INFO_STRUCT;

/*API*/
static RET_INFO api_wan_mode_check_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_wan_mode_check_init();

/*GPI*/
WANMODE gpi_wan_mode_check_get_check_result();

/*TPI*/
extern RET_INFO tpi_wan_mode_check_update_info();
extern RET_INFO tpi_wan_mode_check_struct_init();
extern RET_INFO tpi_wan_mode_check_first_init();
extern RET_INFO tpi_wan_mode_check_action(RC_MODULES_COMMON_STRUCT *var);

extern WANMODE tpi_wan_mode_check_get_check_result();
extern CHECK_RESULT tpi_wan_mode_check_get_check_finish_tag();

extern PIU8 tpi_wan_mode_check_get_exit_tag();
extern RET_INFO tpi_wan_mode_check_set_exit_tag(PIU8 enable);
extern RET_INFO tpi_wan_mode_check_wait_exit();

extern void tpi_wan_mode_check_set_dhcp_discover_add();
extern void tpi_wan_mode_check_set_dhcp_offer_rcv();
extern void tpi_wan_mode_check_set_pppoe_padi_add();
extern void tpi_wan_mode_check_set_pppoe_pado_rcv();
#endif/*__WAN_MODE_CHECK_H_*/
