/*add by lqz 2015-12-17.
*when the system is up then go into this file.
*run_main funciton is rc_main().
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_module.h"
#include "sys_option.h"

/*获取TLV格式中的一个TLV*/
PI8 * sys_module_get_option(PI8 *msg,PIU8 option,PI8 *len)
{
	PI8 *ret = NULL;
	PI8 *p_msg = msg;

	if(NULL == msg)
		return NULL;

	for(p_msg = msg;p_msg != NULL && (p_msg + 1) != NULL && *p_msg != '\0' && *(p_msg+1) != '\0';p_msg = p_msg + *(p_msg+1))
	{
		if(option == *p_msg)
		{
			ret = p_msg;
			if(NULL != len)
			{
				*len = *(p_msg+1);
			}
			break;
		}
	}
	
	return ret;
}

/*依次获取TLV格式中的TLV*/
PI8 * sys_module_get_next_option(PI8 **msg,PI8 *len)
{
	PI8 *ret = NULL;

	if(msg != NULL && *msg != NULL && (*msg + 1) != NULL && *(*msg) != '\0' && *(*msg+1) != '\0')
	{
		ret = *msg;
		if(NULL != len)
		{
			*len = *(*msg+1);
		}
		*msg += *(*msg+1);
	}
	
	return ret;
}

/*将一个消息组装成TLV格式*/
RET_INFO sys_module_set_option(PI8 **msg,PIU8 type,PIU8 len,PI8 *value)
{
	RET_INFO ret = RET_SUC;

	if(NULL == msg || NULL == *msg || NULL == value || 3 > len || RC_TYPE_NONE == type)
	{		
		PI_ERROR(MAIN,"msg IS NULL or value is NULL or msg type is NONE or len is not inrand[%c]!\n",len);
		return RET_ERR;
	}
	
	*(*msg) = type;
	*(*msg+1) = len;
	strncpy(*msg+2,value,len-2);
	*msg += len;
	
	return ret;
}

/*将所有消息组装成TLV格式，形成最终的消息*/
RET_INFO sys_module_msg_2_TLV(PIU8 center,PIU8 id,PI8 *msg,PI8 *tlv_msg)
{
	RET_INFO ret = RET_SUC;
	PIU8 i = 0,center_num = 0;

	if(NULL == tlv_msg || (center <= 0 || center > (MODULE_MAX_NUM-1)))
	{
		PI_ERROR(MAIN,"tlv_msg IS NULL or center is not inrand[%d,%d]!\n",center,MODULE_MAX_NUM - 1);
		return RET_ERR;
	}

	center_num = sys_get_center_num();

	if(NULL != msg)
	{
		for(i = 0;i < center_num;i++)
		{
			if(center == moudle_center_struct_list[i].center)
			{
				if(NULL == moudle_center_struct_list[i].msg_2_tlv_func
					|| RET_ERR == moudle_center_struct_list[i].msg_2_tlv_func(msg,tlv_msg))
				{
					PI_ERROR(MAIN,"msg to tlv fail!\n");
					return RET_ERR;
				}
				break;
			}
		}

		if(i == center_num)
		{
			PI_ERROR(MAIN,"not find center [%d]\n",center);
			ret = RET_ERR;
		}
	}
	
	return ret;
}
