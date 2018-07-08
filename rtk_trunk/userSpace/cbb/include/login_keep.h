#ifndef __LOGIN_KEEP_H__
#define __LOGIN_KEEP_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

RET_INFO tpi_login_keep_action(RC_MODULES_COMMON_STRUCT *var);
RET_INFO tpi_login_keep_first_init();

#endif

