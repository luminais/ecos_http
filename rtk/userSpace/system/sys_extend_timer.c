/*add by lqz 2015-12-17.
*
*
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_timer.h"
#include "sys_extend_timer.h"

static P_DO_TIMER_FUN head;

void sys_extend_do_timer()
{
	P_DO_TIMER_FUN p = head;
	int i = 0;
	while(NULL != p)
	{
		if(DO_TIMER_ON == p->enable && p->fun)
		{
			if((p->before_run_time == 0 && p->next_run_time == 0) || 
				(p->next_run_time - p->before_run_time >= p->sleep_time) ||
				(p->next_run_time < p->before_run_time))
			{
				p->fun();
				p->before_run_time = p->next_run_time;
			}
			p->next_run_time += DO_TIMER_MIN_TIME/2;
		}
		p = p->next;
	}
	
    timeout((timeout_fun *)sys_extend_do_timer, NULL, DO_TIMER_MIN_TIME/2);
	
    return;
}

void sys_extend_do_timer_add(P_DO_TIMER_FUN ptr)
{
    P_DO_TIMER_FUN p = head,p1 = head,node = NULL;

    if(NULL == ptr || NULL == ptr->fun)
    {
        PI_ERROR(MAIN,"ptr or ptr->fun is NULL!\n");
		return;
    }

	if(NULL != p)
	{
		while(NULL != (p->next))
		{
			p1 = p->next;
			if(0 == strncmp(p1->name,ptr->name,DO_TIMER_MAX_NAME))
			{
				if(p1->fun == ptr->fun)
				{	
					if(DO_TIMER_ON != p->enable)
					{
						p1->enable = DO_TIMER_ON;
						p1->before_run_time = 0;
						p1->next_run_time = 0;
						PI_ERROR(MAIN,"timer have add already,but is stop,now start it!\n");
					}
					PI_ERROR(MAIN,"timer have add already!\n");
					return;
				}
				else
				{
					PI_ERROR(MAIN,"timer have add already,but fun is not same,cannot add it,please del it first!\n");
				}
			}
			p = p->next;
		}
	}

	node = (P_DO_TIMER_FUN)malloc(sizeof(DO_TIMER_FUN));

	if(NULL == node)
	{
		PI_ERROR(MAIN,"malloc fail!\n");
		return;		
	}

	memcpy((PI8 *)node,(PI8 *)ptr,sizeof(DO_TIMER_FUN));
	node->next = NULL;

	if(NULL == head)
		head = node;
	else
		p->next = node;
	
    return;
}

void sys_extend_do_timer_del(PI8 *name)
{
    P_DO_TIMER_FUN p = head,free_p = NULL;

    if(NULL == name)
    {
        PI_ERROR(MAIN,"timer name is NULL!cannot del timer\n");
		return;
    }

	if(NULL == p)
    {
        PI_ERROR(MAIN,"head is NULL!cannot del timer(%s)\n",name);
		return;
    }

	if(0 == strncmp(head->name,name,DO_TIMER_MAX_NAME))
	{
		free_p = head;
		head = head->next;
		free(free_p);
		PI_PRINTF(MAIN,"del timer(%s) ok!\n",name);
		return;
	}

	while(NULL != (p->next))
	{
		if(0 == strncmp(head->name,name,DO_TIMER_MAX_NAME))
		{		
			break;
		}
		p = p->next;
	}

	if(NULL == p->next)
	{
        PI_ERROR(MAIN,"del fail,cannot find it(%s)!\n",name);
		return;
	}

	free_p = p->next;

	p->next = (p->next)->next;

	free(free_p);
	
	PI_PRINTF(MAIN,"del timer(%s) ok!\n",name);
		
    return;
}

void sys_extend_do_timer_action(DOTIMERINFO action,PI8 *name)
{
	P_DO_TIMER_FUN p = head;

	if(NULL == name)
	{
		PI_ERROR(MAIN,"timer name is NULL!can not change timer\n");
		return; 	
	}

	while(NULL != p)
	{
		if(0 == strncmp(p->name,name,DO_TIMER_MAX_NAME))
		{		
			break;
		}
		p = p->next;
	}
	
	if(NULL == p)
	{
        PI_ERROR(MAIN,"cannot find it(%s)!\n",name);
		return;		
	}
	
	switch(action)
	{
		case DO_TIMER_OFF:
		case DO_TIMER_ON:
			if(action != p->enable)
			{
				p->enable = action;
				p->before_run_time = 0;
				p->next_run_time = 0;
				PI_PRINTF(MAIN,"%s timer(%s) ok!\n",name,(action==DO_TIMER_ON)?"start":"stop");
			}
			else
			{
				PI_ERROR(MAIN,"timer(%s )is already %s!\n",name,(action==DO_TIMER_ON)?"start":"stop");					
			}
			break;
		default:
			PI_ERROR(MAIN,"action is %d error!\n",action);
			break;
	}
	
    return;
}

void sys_extend_do_timer_show(void)
{
	P_DO_TIMER_FUN p = head;

	printf("\nTIMER:\n");
	
	printf("%-32s \t%s \t%s(10ms) \t%s\n","name","enable","time","funp");

	while(p)
	{
		printf("%-32s \t",p->name);
		printf("%d \t",p->enable);
		printf("%d(%.02fs) \t",p->sleep_time,(float)(p->sleep_time)/100);
		printf("%p \n",p->fun);
		p = p->next;
	}
	
    return;
}

#include "gpio_api.h"
EXTEND_LED_STATUS extend_led_blink_status = STOP_BLINK;
void extend_led_start_blink()
{
	extend_led_on_off(LED_OFF);
	extend_led_blink_status = START_BLINK;

	return;
}

void extend_led_stop_blink()
{
	extend_led_blink_status = STOP_BLINK;

	return;
}

EXTEND_LED_STATUS extend_led_blink()
{
	return extend_led_blink_status;
}