#ifndef __SYS_INIT_SERVICES_H__
#define	__SYS_INIT_SERVICES_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif


struct services_funs
{
	void (*fun)();
};

#endif/*__SYS_INIT_SERVICES_H__*/
