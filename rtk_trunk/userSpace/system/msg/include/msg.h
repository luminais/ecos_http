#ifndef __MSG_H__
#define __MSG_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

#define MAX_MBOX_MSG_WAIT_TIME (200)
#define MAX_MBOX_MSG_WAIT_SLEEP_TIME (2)
#define MAX_MBOX_MSG_WAIT_NUM (MAX_MBOX_MSG_WAIT_TIME/MAX_MBOX_MSG_WAIT_SLEEP_TIME)

#define MAX_MBOX_MSG_NUM (10)
#define MAX_MBOX_MSG_LEN MODULE_OPTION_MAX_LEN

struct msg_struct
{
	cyg_handle_t mbox_msg_id;
	cyg_mbox mbox;
	PIU8 mbox_tag;
	PI8 msg_info[MAX_MBOX_MSG_NUM][MAX_MBOX_MSG_LEN];
};

#define MSG_CENTER_ID(CENTER,ID) ((((PIU32)(CENTER))&0x00ff)<<8)|((((PIU32)(ID))&0x00ff)<<0)

#define MSG_CENTER(MSG) (PIU8)(((PIU32)(MSG)>>8)&0x00ff)

#define MSG_ID(MSG) (PIU8)(((PIU32)(MSG)>>0)&0x00ff)

extern struct msg_struct mbox_msg;
extern RET_INFO (*sys_msg_init)();
extern RET_INFO (*sys_msg_loop)();
extern RET_INFO msg_init();
extern RET_INFO msg_main();
void msg_waitback(PIU8 center_id,PIU8 module_id,PIU32 module_delay,PIU32 module_extra_delay);
extern RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg);

#endif/*__MSG_H__*/
