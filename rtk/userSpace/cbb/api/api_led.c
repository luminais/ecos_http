#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "led.h"

static RET_INFO api_led_handle(RC_MODULES_COMMON_STRUCT *var);
static RET_INFO api_led_init();

static RET_INFO api_led_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(TPI,"var is null!\n");
		return RET_ERR;
	}
	
    return tpi_led_action(var);
}

static struct rc_msg_ops rc_led_ops[] =
{
    {
        .intent_module_id = RC_LED_MODULE,
		.type = INTENT_NONE,
        .ops = api_led_handle,
    },
};

static RET_INFO api_led_init()
{
	tpi_led_struct_init();
	tpi_led_first_init();
    rc_register_module_msg_opses(rc_led_ops,RC_LED_MODULE,ARRAY_SIZE(rc_led_ops));
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_LED_MODULE,"rc_led_time_check",api_led_init,NULL);




