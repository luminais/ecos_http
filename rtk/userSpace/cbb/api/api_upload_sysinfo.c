#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif
static RET_INFO api_upload_sysinfo_handle(RC_MODULES_COMMON_STRUCT *var)
{
	if(NULL == var)
	{
		PI_ERROR(API,"var is null!\n");
		return RET_ERR;
	}

	return tpi_upload_sysinfo_action(var);
}

 
static struct rc_msg_ops rc_upload_sysinfo_ops[] =
{
    {
        .intent_module_id = RC_UPLOAD_SYSINFO_MODULE,
		.type = INTENT_NONE,
        .ops = api_upload_sysinfo_handle,
    },
};


static RET_INFO api_upload_sysinfo_init()
{
    tpi_upload_sysinfo_init();
    rc_register_module_msg_opses(rc_upload_sysinfo_ops,RC_UPLOAD_SYSINFO_MODULE,ARRAY_SIZE(rc_upload_sysinfo_ops));
    
	return RET_SUC;
}

RC_MODULE_REGISTER(RC_UPLOAD_SYSINFO_MODULE,"rc_upload_sysinfo",api_upload_sysinfo_init,NULL);

