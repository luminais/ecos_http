#ifndef __SYS_MODULE_H__
#define __SYS_MODULE_H__

#ifndef __RC_MODULE_H__
#include "rc_module.h"
#endif

/*这里表示能够接收消息的消息ID索引*/
typedef enum
{
    MODULE_RC = 1,
    MODULE_MAX_NUM,
} MODULE_CENTER_ID;

struct moudle_center_struct
{
	MODULE_CENTER_ID center;/*接收消息中心ID 0-255*/
	PI8  name[PI_BUFLEN_32];/*接收消息名称，主线程名称更改为rc，1-32*/
    RET_INFO (*init)(); /* 接收消息中心初始化函数，系统启动完成后启动应用层调用*/
	RET_INFO (*msg_2_tlv_func)(PI8 *msg,PI8 *tlv_msg);/*该模块下将MSG转换成TLV格式函数*/
	RET_INFO (*rcv_msg_handle)(PIU8 id,PI8 *msg);/*该模块下处理消息的函数*/
	void (*callback)(PIU8 module_id,PIU32 module_delay,PIU32 module_extra_delay);/*回调函数*/
};

extern struct moudle_center_struct moudle_center_struct_list[];
extern PIU8 sys_get_center_num();
#endif/*__SYS_MODULE_H__*/
