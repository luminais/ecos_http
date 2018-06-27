#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

extern PIU64 _RC_DEBUG_TAG_;

#define RC_MODULE_UNIN_DEBUG_TAG(id) (id < 0 || id >= 64)

#define RC_MODULE_DEBUG_EACH(id) (((PIU8)(_RC_DEBUG_TAG_>>id))&0x01)

#define RC_MODULE_DEBUG_ENABLE(id) ((RC_MODULE_ID_UNINRAND(id))?0:RC_MODULE_DEBUG_EACH(id))

#define RC_MODULE_DEBUG(id,PI_NAME,fmt, arg...) \
do{ \
	if (RC_MODULE_DEBUG_ENABLE(id)){\
		printf(CON_COLOR_GREEN "[%s->%s->%d]:" CON_COLOR_END fmt,PI_NAME, __FUNCTION__, __LINE__, ##arg);\	
	}\
}while(0);

extern RET_INFO tpi_debug_show_info();
extern RET_INFO tpi_debug_action(PIU8 debug_id);
#endif
