#ifndef __SYS_OPTION_H__
#define __SYS_OPTION_H__

#ifndef __PI_COMMON_H__
#include "pi_common.h"
#endif

typedef enum {
	OP_DEFAULT = 0,
	OP_START, 			 /* 启动 */
	OP_STOP, 			 /* 停止 */
	OP_RESTART,		 /* 重启 */
	OP_ADD, 			 /* 增加 */
	OP_DELETE,		 	 /* 删除*/
	OP_UPDATE, 			 /* 更新 */
	OP_CLEAR, 			 /* 清除 */
	COMMON_MSG_MAX 
} MODULE_COMMON_OP_TYPE;

#define MODULE_TYPE_MIN_LEN 3
#define MODULE_TYPE_MAX_LEN 128

#define MODULE_OPTION_MIN_LEN 6
#define MODULE_OPTION_MAX_LEN 256       //lq后续如果需要增加消息或者更新消息，需要注意消息的长度

typedef struct
{
	PIU8 type;
	PIU8 len;
	PI8 *value;
} MODULE_MSG_OPTION;

#define RC_MODULE_OP_INRAND(op) (op < OP_DEFAULT || op >= COMMON_MSG_MAX)

typedef enum
{
    VAR_TYPE_NONE = 0,
    VAR_TYPE_STR,
    VAR_TYPE_NUM_8,
    VAR_TYPE_NUM_32,
    VAR_TYPE_NUM_64,
} VAR_TYPE;

typedef struct
{
    char *var;
    void *value;
	PIU8 type;
    VAR_TYPE len_type;
} COMMON_VAR_ITEM;

extern PI8 * sys_module_get_option(PI8 *msg,PIU8 option,PI8 *len);
extern PI8 * sys_module_get_next_option(PI8 **msg,PI8 *len);
#endif/*__SYS_OPTION_H__*/
