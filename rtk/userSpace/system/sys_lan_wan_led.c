#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pi_common.h"
#include "gpio_api.h"

unsigned int old_status[5] = {0, 0, 0, 0, 0};
unsigned int link_stat[5] = {0, 0, 0, 0, 0};
unsigned int upgreade_ctr_lan_led = 0;
extern unsigned g_set_lan_wan_led;

#ifdef __CONFIG_ATE__
unsigned int g_ate_ctl_led; 
#endif

void port_link_update()
{
    unsigned int i;

    extern unsigned char tenda_show_phy_stats(int port);

    for (i = 0; i < 5; i ++) {
        link_stat[i] = tenda_show_phy_stats(i);
    }
}

void lan_led_timeout(unsigned int port)
{
    unsigned int i, lan_link = 0;
    unsigned int lan_led_blink_count = 2;

    extern void set_lan_led_on_off(unsigned int val);

    for (i = 0; i < 5; i ++) {
        if (((1 << i) & __CONFIG_LAN_PORT_MASK__)
            && (link_stat[i] != 0)) {
                lan_link = 1;
                break;
        }
    }
		
    if (lan_link) {
        set_lan_led_on_off(1);
    } else {
        set_lan_led_on_off(0);
    }
    cyg_thread_delay(10);
    if ((link_stat[port] != old_status[port]) 
        && (link_stat[port] == 1)) { 
            while(lan_led_blink_count > 0) {
                set_lan_led_on_off(0);
                cyg_thread_delay(20);//0.2s
                set_lan_led_on_off(1);
                cyg_thread_delay(20);
                lan_led_blink_count --;
           }
    }
}

void lan_wan_led_timer()
{
    unsigned int i;
	static int set_flag = 0;

	/*如果设置了智能省电:常关/定时关闭时,不需要轮询lan_linkStatus;
	 *智能省电模块 led打开：g_set_lan_wan_led= 0；
	 *智能省电模块 led关闭：g_set_lan_wan_led= 1；
	 *智能省电模块关不灭lan口灯，初步定位为线程间同步问题，暂时添加规避措施
	*/
#ifdef	__CONFIG_ATE__
    /*
	  产测控制lan口灯亮灭，lan口状态不再轮寻
	*/
	if (g_ate_ctl_led)
		return;
#endif
    if (upgreade_ctr_lan_led)
        return;
 
	if(1 == set_flag && 1 == g_set_lan_wan_led)
	{
		return;
	}
	if (g_set_lan_wan_led) {
		set_lan_led_on_off(0);
		set_flag = 1;
		return;	
    }
	set_flag = 0;
		
    port_link_update();

    for (i = 0; i < 5; i ++) {
        if ((1 << i) & __CONFIG_LAN_PORT_MASK__) {
            lan_led_timeout(i);
        }
    }

    memcpy(old_status, link_stat, sizeof(link_stat));
}
