/*
*
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cyg/hal/drv_api.h>

#include "sys_module.h"
#include "sys_option.h"
#include "msg.h"

/*钩子函数的挂载*/
RET_INFO (*sys_msg_init)() = msg_init;
RET_INFO (*sys_msg_loop)() = msg_main;

/*消息实体*/
struct msg_struct mbox_msg;

/*互斥锁*/
static cyg_drv_mutex_t msg_mutex;

/*公共函数*/
extern int run_sh(char *cmd, char *argv[], char *envp[]);

/*初始化*/
/*this function is only by user_main().this function is used for init mbox*/
RET_INFO msg_init()
{
	RET_INFO ret = RET_SUC;

	memset(&mbox_msg,0x0,sizeof(mbox_msg));
	cyg_mbox_create(&(mbox_msg.mbox_msg_id), &(mbox_msg.mbox));
	cyg_drv_mutex_init(&msg_mutex);
	PI_DEBUG(MSG,"rc_msg init ok!\n");

	return ret;
}

/*msg call back*/
void msg_waitback(PIU8 center_id,PIU8 module_id,PIU32 module_delay,PIU32 module_extra_delay)
{
	PIU8 i = 0;
	
	if(center_id < MODULE_RC || center_id >= MODULE_MAX_NUM)
		return;

	if(moudle_center_struct_list[center_id - 1].callback)
	{
		moudle_center_struct_list[center_id - 1].callback(module_id,module_delay,module_extra_delay);
	}
	
	return;
}

/*收消息*/
RET_INFO msg_rcv_msg_2_mbox(PIU8 *center,PIU8 *id,PI8 *msg)
{
	RET_INFO ret = RET_SUC;
	PIU32 mbox_info = 0;
	PIU8 i = 0;

	if(NULL == msg)
	{
		PI_ERROR(MSG,"msg rcv error!\n");
		return RET_ERR;
	}
	
	mbox_info = ((PIU32)cyg_mbox_get(mbox_msg.mbox_msg_id) & 0xffff);
	
	cyg_drv_mutex_lock(&msg_mutex);
	
	*center = MSG_CENTER(mbox_info);
	*id = MSG_ID(mbox_info);
	strncpy(msg,mbox_msg.msg_info[0],strlen(mbox_msg.msg_info[0]));

	memset(mbox_msg.msg_info[0],0x0,MAX_MBOX_MSG_LEN);
	for(i = 1;i < mbox_msg.mbox_tag;i++)
	{
		strcpy(mbox_msg.msg_info[i - 1],mbox_msg.msg_info[i]);
	}
	memset(mbox_msg.msg_info[mbox_msg.mbox_tag-1],0x0,MAX_MBOX_MSG_LEN);
	mbox_msg.mbox_tag--;
	
	cyg_drv_mutex_unlock(&msg_mutex);
	
	return ret;
}

RET_INFO msg_rcv(PIU8 *center,PIU8 *id,PI8 *msg)
{
	return msg_rcv_msg_2_mbox(center,id,msg);
}

/*发消息*/
RET_INFO msg_send_msg_2_mbox(PIU8 center,PIU8 id,PI8 *msg)
{	
	RET_INFO ret = RET_SUC;
	PIU8 i = 0;
	PIU8 msg_put_tag = 0;
	PIU32 mbox_info = 0;
	PI8 msg_info[MODULE_OPTION_MAX_LEN] = "";

	if((center <= 0 || center > (MODULE_MAX_NUM-1)))
	{	
		PI_ERROR(MSG,"id :%c error!\n",center);
		return RET_ERR;		
	}
	
	if(RET_ERR == sys_module_msg_2_TLV(center,id,msg,msg_info))
	{
		PI_ERROR(MSG,"send msg [%s] fail!\n",msg == NULL?"NULL":msg);
		return RET_ERR;
	}

	mbox_info = MSG_CENTER_ID(center,id);

	PI_PRINTF(MSG,"center:%08x\n",mbox_info);

	for(i = 0;i < MAX_MBOX_MSG_WAIT_NUM;i++)
	{
		if(!msg_put_tag && mbox_msg.mbox_tag < MAX_MBOX_MSG_NUM)
		{			
			/*消息内容必须要在放入消息盒子之前放入，否则会出现收消息更快，导致消息内容出错*/
			cyg_drv_mutex_lock(&msg_mutex);
			memset(mbox_msg.msg_info[mbox_msg.mbox_tag],0x0,MAX_MBOX_MSG_LEN);
			strncpy(mbox_msg.msg_info[mbox_msg.mbox_tag],msg_info,strlen(msg_info));
			mbox_msg.mbox_tag++;
			cyg_drv_mutex_unlock(&msg_mutex);
			msg_put_tag = 1;
		}
		if(msg_put_tag && cyg_mbox_tryput(mbox_msg.mbox_msg_id, (void *)mbox_info))
		{
			PI_PRINTF(MSG,"send msg [%s] success!\n",msg == NULL?"NULL":msg);
			break;
		}
		cyg_thread_delay(MAX_MBOX_MSG_WAIT_SLEEP_TIME);
	}
	
	if(msg_put_tag && MAX_MBOX_MSG_WAIT_NUM == i)
	{
		/*如果消息放入消息盒子失败，将消息内容清除，丢掉该消息*/
		cyg_drv_mutex_lock(&msg_mutex);
		memset(mbox_msg.msg_info[mbox_msg.mbox_tag - 1],0x0,MAX_MBOX_MSG_LEN);
		mbox_msg.mbox_tag--;	
		cyg_drv_mutex_unlock(&msg_mutex);
		ret = RET_ERR;
		PI_PRINTF(MSG,"send msg [%s] error!\n",msg == NULL?"NULL":msg);
	}

	return ret;
}

RET_INFO msg_send(PIU8 center,PIU8 id,PI8 *msg)
{
	if((center <= 0 || center > (MODULE_MAX_NUM-1)))
	{	
		PI_ERROR(MSG,"id :%c error!\n",center);
		return RET_ERR;		
	}
	
	return msg_send_msg_2_mbox(center,id,msg);
}

/*主函数*/
RET_INFO msg_rcv_handle()
{
	RET_INFO ret = RET_SUC;
	PIU8 center = 0,id = 0,i = 0,center_num = 0;
	PI8 msg[MAX_MBOX_MSG_LEN] = {0};	
	PI8 center_num_str[PI_BUFLEN_4] = {0};
	PI8 center_str[PI_BUFLEN_4] = {0};
	PI8 id_str[PI_BUFLEN_4] = {0};
	char *argv[] = {"msg",center_num_str,center_str,id_str, NULL};
	char *env[] = {msg,NULL};

	center_num = sys_get_center_num();
	sprintf(center_num_str,"%d",center_num);
	
	if(RET_SUC == msg_rcv(&center,&id,msg))
	{	
		sprintf(center_str,"%d",center);
		sprintf(id_str,"%d",id);
		run_sh(argv[0], argv, env);
	}
	return ret;
}

RET_INFO msg_main()
{
	while(1)
	{
		msg_rcv_handle();
	}
	
	return RET_ERR;
}
