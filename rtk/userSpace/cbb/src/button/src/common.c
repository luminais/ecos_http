/*************************************************************************
  > Copyright (C) 1998-2016, Tenda Tech. Co., All Rights Reserved.
  > File Name: common.c
  > Description: 
  > Author: ZhuHuan
  > Mail: zhuhuan_IT@outlook.com
  > Version: 1.0
  > Created Time: Friday 2016-07-01 08:40 CSThandle Function List: 

  > History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#include <stdio.h>

#include "sys_timer.h"
#include "button.h"

#ifndef ZH_DEBUG
#define ZH_DEBUG(format, string...) printf("func=%s;line=%d; " format, __func__, __LINE__,  ##string)
#endif

/* Function declaration */
extern int tenda_button_status(unsigned int gpio);

extern BUTTON reset_button_desc;
#ifdef __CONFIG_WIFI_BUTTON_RTK__
extern BUTTON wifi_button_desc;
#endif
#ifdef __CONFIG_WPS_RTK__
extern BUTTON wps_button_desc;
#endif
static PIU8 sys_button_check(PIU8 gpio);

static P_BUTTON button_desc[] = {
    &reset_button_desc,
#ifdef __CONFIG_WIFI_BUTTON_RTK__
    &wifi_button_desc,
#endif
#ifdef __CONFIG_WPS_RTK__
    &wps_button_desc,
#endif
    NULL
};

static PIU8 ate_mode = 0;

void init_button_services(void)
{
    DO_TIMER_FUN timer;

    memset(&timer,0x0,sizeof(DO_TIMER_FUN));
	strcpy(timer.name,BUTTON_TIMER);
    timer.enable = DO_TIMER_ON;
    timer.sleep_time = DO_TIMER_MIN_TIME;
    timer.fun = button_polling_timer;
    sys_do_timer_add(&timer);

    return ;
}

void button_polling_timer(void)
{
    BUTTON **p = button_desc;
    PIU8 button_status = 0;

    while(NULL != *p)
    {
        /* 读取按键状态，按下还是松开 */
        button_status = sys_button_check((*p)->gpio);

        if(BUTTON_PUSH_DOWN == button_status)   // 按键被释放
        {
            /* 
             * @ Developing notes, recorded by zhuhuan on 2016-06-28 10:23 @
             * 按键松开后的操作:
             * 1.首先需要处理按键类型为松开生效的按键，满足以下条件：
             *  1)按键类型为松开生效；
             *   2)该按键按下的的时间在预定的时间以内；
             *   3)当前不是处于产测模式。
             * 2.完成清除工作，因为本次按键记录已经结束，原来的记录需要清除，保证下次
             * 按键被再次按下时可以正常工作。
             */
            if((UNLOCKED_EFFECT == (*p)->trigger_type) && \
                ((*p)->count >= (*p)->min_time) && ((*p)->count <= (*p)->max_time) && \
                (1 != ate_mode))
            {
                (*p)->handle();
            }

            (*p)->count = 0;
            (*p)->is_handled = 0;
        }
        else    // 按键被按下
        {
           /* 
            * @ Developing notes, recorded by zhuhuan on 2016-06-28 10:17 @
            * 按键被按下后的操作：
            * 1.记录该按键被按下的次数加1，这个次数就代表了按键按下的时间，单位为1s；
            * 2.对于按下生效的按键来说，执行相应的处理函数需要满足一下三点:
            *   1)按键类型为按下生效；
            *   2)该按键按下的的时间在预定的时间以内；
            *   3)该次按下是否已经处理，保证在预定的时间内按键的处理函数只执行1次；
            *   4)当前不是处于产测模式。
            * PS：对于按键复用来说，不能通过按下就生效来实现，因为软件无法判断用户还
            * 需要按多少秒，所以按键复用的动作是在按键被释放的时候执行。
            */
            (*p)->count++;

            ZH_DEBUG("%s button pressed [%d s]\n", (*p)->name, (*p)->count);
            if((LOCKED_EFFECT  == (*p)->trigger_type) && \
               ((*p)->count >= (*p)->min_time) && ((*p)->count <= (*p)->max_time) && \
               (0 == (*p)->is_handled) && \
               (1 != ate_mode))
            {
                (*p)->handle();
                (*p)->is_handled = 1;
            }
        }

        ++p;
    }

    return ;
}

static PIU8 sys_button_check(PIU8 gpio)
{
	return (PIU8)tenda_button_status(gpio);
}

/*ate模块下调用*/
void sys_button_change_ate_mode(PIU8 status)
{
	ate_mode = status;
    return;
}

PIU8 sys_button_get_ate_result(PIU8 gpio)
{
    BUTTON **p = button_desc;
	PIU8 ret = 0;

    while(NULL != *p)
	{
		if((*p)->gpio == gpio)
		{
			if((*p)->count >= 1)
			{
				ret = 1;
			}
			break;	
		}
		++p;
	}
	return ret;
}

PIU8 sys_button_get_button_result(void)
{
    BUTTON **p = button_desc;
	PIU8 ret = 0;

	button_polling_timer();
	
    while(NULL != *p)
	{
		if((*p)->count >= 1)
		{
			ret = 1;
			break;
		}
		++p;
	}
	return ret;
}
