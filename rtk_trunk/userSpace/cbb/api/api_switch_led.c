#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "switch_led.h"

static RET_INFO api_switch_led_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_switch_led_init();

static RET_INFO api_switch_led_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_switch_led_action(var);
}

static struct rc_msg_ops rc_switch_led_ops[] =
{
    {
        .intent_module_id = RC_SWITCH_LED_MODULE,
		.type = INTENT_NONE,
        .ops = api_switch_led_handle,
    },
};

static RET_INFO api_switch_led_init()
{
	tpi_switch_led_struct_init();
	tpi_switch_led_first_init();
    rc_register_module_msg_opses(rc_switch_led_ops,RC_SWITCH_LED_MODULE,ARRAY_SIZE(rc_switch_led_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_SWITCH_LED_MODULE,"rc_switch_led",api_switch_led_init,NULL);




