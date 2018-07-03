/*
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "sys_timer.h"

PIU64 _RC_DEBUG_TAG_;

RET_INFO tpi_debug_show_info()
{
    RET_INFO ret = RET_SUC;
	rc_module_node_t *p_entry = NULL;
	rc_module_node_t *rc_module = NULL;
    struct rc_msg_ops *elem = NULL;
	
	PI8 rc_index[PI_BUFLEN_64];
	PIU8 rc_max = 0,i = 0;

	/*遍历模块信息数据段*/
	memset(rc_index,-1,sizeof(rc_index));
	RC_MODULE_LIST_SCAN(rc_module)
	{
		rc_index[rc_module->id] = rc_max;
		rc_max++;
	}

	for(i = 0;i < PI_BUFLEN_64;i++)
	{
		if(-1 != rc_index[i])
		{
			p_entry = __rc_module_tab__ + rc_index[i];
	        printf("id:%d\tname:%s\n",p_entry->id,p_entry->name);
	        printf("\tps_num:%d\tops_module(id):",p_entry->ops_num);
			if(p_entry->ops_num > 0)
	        {
	            list_for_each_entry(elem, &p_entry->ops_head, list)
	            {
					rc_module = rc_find_module_index(elem->intent_module_id);
	                printf("[%d,%s,%s] ",elem->intent_module_id,rc_module->name,elem->type == INTENT_PREV?"PREV":(elem->type == INTENT_NEXT?"NEXT":"NONE"));
	            }
	        }	
	        printf("\n");
		}
	}

	sys_do_timer_show();
	
    return ret;
}

RET_INFO tpi_debug_action(PIU8 debug_id)
{
    RET_INFO ret = RET_SUC;

	if(RC_MODULE_ID_UNINRAND(debug_id) || RC_MODULE_UNIN_DEBUG_TAG(debug_id))
    {
		PI_ERROR(TPI,"module_id[%d] is not inrand [%d,%d]/[0,63]!\n",debug_id,RC_DBG_MODULE,RC_MAX_MODULE_NUM-1);
        ret = RET_ERR;
    }
    else
    {
        if(RC_MODULE_DEBUG_EACH(debug_id))
        {
			_RC_DEBUG_TAG_ &= ~((PIU64)1<<debug_id);
			PI_DEBUG(TPI,"module_id:%d,turn off debug!\n",debug_id);
        }
        else
        {
			_RC_DEBUG_TAG_ |= ((PIU64)1<<debug_id);
			PI_DEBUG(TPI,"module_id:%d,turn on debug!\n",debug_id);
        }
    }

    return ret;
}