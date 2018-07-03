/*add by lqz 2015-12-17.
*when the system is up then go into this file.
*run_main funciton is rc_main().
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rc_module.h"

CYG_HAL_TABLE_BEGIN(__rc_module_tab__, rc_module_tab);/*将__rc_module_tab__指向rc_module_tab数据段首地址*/
CYG_HAL_TABLE_END(__rc_module_tab_end__, rc_module_tab);/*将__rc_module_tab_end__指向rc_module_tab数据段尾地址*/

static RC_MODULES_COMMON_STRUCT rc_modules_var;

static COMMON_VAR_ITEM rc_var_table[] =
{
    {"op",(void *)&rc_modules_var.op,RC_TYPE_OP,VAR_TYPE_NUM_8},
    {"debug_id",(void *)&rc_modules_var.debug_id,RC_TYPE_DEBUG_ID,VAR_TYPE_NUM_8},
     {"wlan_ifname",(void *)&rc_modules_var.wlan_ifname,RC_TYPE_WLAN_IFNAME,VAR_TYPE_STR},
    {"string_info",(void *)&rc_modules_var.string_info,RC_TYPE_STRING_INFO,VAR_TYPE_STR},
};

void rc_callbak(PIU8 module_id,PIU32 module_delay,PIU32 module_extra_delay)
{
	PIU32 i = 0;
	rc_module_node_t *rc_module = NULL;

	if(RC_MODULE_ID_UNINRAND(module_id))
    {
        PI_ERROR(RC,"module_id = %d is not inrand [%d,%d]\n",module_id,RC_DBG_MODULE,RC_MAX_MODULE_NUM-1);
        return;
    }

	if(NULL == (rc_module = rc_find_module_index(module_id)))
	{
        PI_ERROR(RC,"not found module entry:%d\n",module_id);
        return;
    }
	
	for(i = 0;i < module_delay;i++)
	{
		cyg_thread_delay(10);

		if(NULL == rc_module->callbak)
			break;
		
		if(MODULE_COMPLETE == rc_module->callbak(MODULE_CALLBACK_GET))
		{
			rc_module->callbak(MODULE_CALLBACK_REINIT);
			break;
		}
	}
	
	for(i = 0;i < module_extra_delay;i++)
	{
		cyg_thread_delay(10);
	}
	
	return;
}

/*
*RC模式下寻找各个消息的类型
*/
RC_TYPE_ID rc_msg_get_option_type(PI8 *op_name)
{
	RC_TYPE_ID type = RC_TYPE_NONE;
	PIU8 i = 0;

	if(NULL == op_name)
		return RC_TYPE_NONE;
	
    for(i = 0;i < ARRAY_SIZE(rc_var_table);i++)
    {
		if(0 == strcmp(op_name,rc_var_table[i].var))
		{
			type = rc_var_table[i].type;
			break;
		}
	}
	
	return type;
}

/*
*RC模式下将msg转换成TLV格式
*msg = "op=1,debug_id=1,string_info=123"
*tlv_msg = 431 531 65123 T-L-V格式存储
*/
RET_INFO rc_msg_2_tlv(PI8 *msg,PI8 *tlv_msg)
{
	RET_INFO ret = RET_SUC;	
    PI8 *p = NULL,*p_msg = msg,*p_tlv_msg = tlv_msg;
    PI8 var[PI_BUFLEN_32] = {0};
    PI8 value[PI_BUFLEN_128] = {0};
	RC_TYPE_ID type = RC_TYPE_NONE;


	if(NULL == msg || NULL == tlv_msg)
		return RET_ERR;
	
	p = strtok(p_msg,",");
    if ( p != NULL)
    {
        sscanf(p,"%[^=]=%s",var,value);
        PI_PRINTF(RC,"key = %s,value = %s\n",var,value);
        if(RC_TYPE_NONE == (type = rc_msg_get_option_type(var)) || RET_ERR == sys_module_set_option(&p_tlv_msg,type,strlen(value)+2,value))
        {
			PI_ERROR(RC,"msg error [%s]\n",msg);
			return RET_ERR;
		}
		
		while( NULL != (p = strtok(NULL,",")))
		{
			memset(var,0x0,sizeof(var));
			memset(value,0x0,sizeof(value));
			sscanf(p,"%[^=]=%s",var,value);
			PI_PRINTF(RC,"key = %s,value = %s\n",var,value);
			if(RC_TYPE_NONE == (type = rc_msg_get_option_type(var)) || RET_ERR == sys_module_set_option(&p_tlv_msg,type,strlen(value)+2,value))
			{
				PI_ERROR(RC,"msg error [%s]\n",msg);
				ret = RET_ERR;
				break;
			}
		}
    }
	
	return ret;
}

/*RC模块将消息按照模块ID发送给对应的模块进行处理*/
void rc_module_msg_handle(PIU8 module_id,RC_MODULES_COMMON_STRUCT *var)
{
    struct rc_msg_ops *elem = NULL;
    rc_module_node_t *rc_module = NULL;

	if(NULL == var)
    {
        return;
    }
	
	if(RC_MODULE_ID_UNINRAND(module_id))
    {
        PI_ERROR(RC,"module_id = %d is not inrand [%d,%d]\n",module_id,RC_DBG_MODULE,RC_MAX_MODULE_NUM-1);
        return;
    }

	if(NULL == (rc_module = rc_find_module_index(module_id)))
	{
        PI_ERROR(RC,"not found module entry:%d\n",module_id);
        return;
    }

    PI_PRINTF(RC,"start handle module[%d,%s]'s msg,op=%d.debug_id=%d.wlan_ifname=%s.string_info=%s\n",
             rc_module->id,rc_module->name,var->op,var->debug_id,var->wlan_ifname,var->string_info);

	if(RC_MODULE_OP_INRAND(var->op))
	{
		PI_ERROR(RC,"op[%d] is not inrand [%d,%d]\n",var->op,OP_DEFAULT,COMMON_MSG_MAX - 1);
		return;
	}
	
    /* 遍历依赖该模块的消息处理函数 */
    list_for_each_entry(elem,&rc_module->ops_head,list)
    {
        if(elem->ops)
        {
            elem->ops(var);
        }
    }
	return;
}

/*RC模块将MSG转换成消息数组*/
RET_INFO rc_msg_2_var(MODULE_MSG_OPTION *option,RC_MODULES_COMMON_STRUCT *var)
{
	RET_INFO ret = RET_SUC;
	PIU8 i = 0;

	if(NULL == option || NULL == var || NULL == option->value || option->len <= 2)
		return RET_ERR;

	for(i = 0;i < ARRAY_SIZE(rc_var_table);i++)
	{
		if(rc_var_table[i].type == option->type)
		{
            switch(rc_var_table[i].len_type)
            {
                case VAR_TYPE_STR:			
					strncpy((PI8 *)rc_var_table[i].value,option->value,(option->len)-2);
					break;
                case VAR_TYPE_NUM_8:
					*((PI8 *)rc_var_table[i].value) = (PIU8)atoi(option->value);
					break;
				case VAR_TYPE_NUM_32:
					*((PI8 *)rc_var_table[i].value) = (PIU32)atol(option->value);
					break;
				case VAR_TYPE_NUM_64:
					*((PI8 *)rc_var_table[i].value) = (PIU64)atof(option->value);
					break;
                default:
                    PI_ERROR(RC,"invalid var type:%d\n",rc_var_table[i].len_type);
                    break;
            }
		}
	}

	return ret;
}

/*RC模块接收处理消息的函数，MSG格式为T-L-V格式*/
RET_INFO rc_rcv_msg_handle(PIU8 module_id,PI8 *msg)
{
	RET_INFO ret = RET_SUC;
	PI8 *msg_str = msg;
	PI8 *str = NULL;
	MODULE_MSG_OPTION option;

	if(RC_MODULE_ID_UNINRAND(module_id))
	{		
        PI_ERROR(RC,"module_id = %d is not inrand [%d,%d]\n",module_id,RC_DBG_MODULE,RC_MAX_MODULE_NUM-1);
		return RET_ERR;
	}
	
	memset(&rc_modules_var,0x0,sizeof(rc_modules_var));

	if(NULL != msg)
	{		
		for(;NULL != (str = sys_module_get_next_option(&msg_str,&(option.len)));)
		{
			option.value = str + 2;
			option.type = *str;
			
			if(RET_ERR == rc_msg_2_var(&option,&rc_modules_var))
			{
				PI_ERROR(RC,"msg[type:%d] is wrong",option.type);
				ret = RET_ERR;
				goto finish;
			}
		}
	}

	rc_module_msg_handle(module_id,&rc_modules_var);

finish:
    return ret;
}

/*寻找对应模块的位置*/
rc_module_node_t * rc_find_module_index(RC_MODULE_ID id)
{
	rc_module_node_t *rc_module = NULL;
	
	if(RC_MODULE_ID_UNINRAND(id))
		return NULL;

	RC_MODULE_LIST_SCAN(rc_module)
	{
		if(rc_module->id == id)
			break;
	}

	if(&__rc_module_tab_end__ == rc_module)
	{
		PI_ERROR(RC,"find module [%d] fail!\n",id);
		return NULL;
	}

	return rc_module;
}

/*RC接收消息中心下的初始化单个执行链函数*/
RET_INFO rc_register_module_msg_ops(rc_module_node_t *p_entry,struct rc_msg_ops *reg)
{
	RET_INFO ret = RET_SUC;
	struct list_node *p;

	if(NULL == p_entry || NULL == reg)
		return RET_ERR;
	
	if(RC_MODULE_ID_UNINRAND(reg->intent_module_id))
    {
        PI_ERROR(RC,"invalid intent module id:%d\n",reg->intent_module_id);
        return RET_ERR;
    }

	switch(reg->type)
	{
		case INTENT_NONE:
			list_append((struct list_node *)reg,&p_entry->ops_head);
			break;
		case INTENT_PREV:
			p = &p_entry->ops_head;
			while(p->next != &p_entry->ops_head)
			{
				p = p->next;
			}			
			list_append((struct list_node *)reg,p->next);
			break;
		case INTENT_NEXT:
			p = &p_entry->ops_head;
			while(p->next != p_entry->ops_head.prev)
			{
				p = p->next;
			}		
			list_append((struct list_node *)reg,p->next);
			break;
		default:	
			PI_ERROR(RC,"invalid type:%d\n",reg->type );
			return RET_ERR;
	}
	
    p_entry->ops_num ++;
    return ret;
}

/*RC接收消息中心下的初始化模块所有的执行链函数*/
RET_INFO rc_register_module_msg_opses(struct rc_msg_ops *reg,RC_MODULE_ID id,PIU8 n)
{
	RET_INFO ret = RET_SUC;
	rc_module_node_t *rc_module;
	PIU8 i = 0;

	if(NULL == reg || RC_MODULE_ID_UNINRAND(id) || n <= 0)
		return RET_ERR;

	if(NULL == (rc_module = rc_find_module_index(id)))
	{
		PI_ERROR(RC,"register rc module fail!\n");
		return RET_ERR;
	}

	rc_module->ops_num = 0;
	
	for(i = 0;i < n;i++)
	{
		if(RET_ERR == rc_register_module_msg_ops(rc_module,&reg[i]))
		{	
			ret = RET_ERR;
            PI_ERROR(RC,"register msg ops error.\n");
            break;
		}
	}
	
	return ret;
}

/*this function is only by user_main().this function is used for init rc module*/
RET_INFO rc_module_init()
{
	RET_INFO ret = RET_SUC;
	rc_module_node_t *rc_module = NULL;

	PI8 rc_index[PI_BUFLEN_64];
	PIU8 rc_max = 0,i = 0;

	/*遍历模块信息数据段，初始化各个模块*/
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
			rc_module = __rc_module_tab__ + rc_index[i];
			if(NULL != (rc_module->init))
			{
				INIT_LIST_HEAD(&(rc_module->ops_head));
				if(RET_SUC !=  rc_module->init())
				{
					PI_ERROR(RC,"module : %s init fail!\n", rc_module->name);
				}
			}
			else
			{
				PI_ERROR(RC,"module : %s don not have init fun!\n", rc_module->name);
			}
		}
	}	

	return ret;
}
