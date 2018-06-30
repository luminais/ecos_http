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
        set_lan_led_on_off(LED_ON);
    } else {
        set_lan_led_on_off(LED_OFF);
    }
    cyg_thread_delay(10);
    if ((link_stat[port] != old_status[port]) 
        && (link_stat[port] == 1)) { 
            while(lan_led_blink_count > 0) {
                set_lan_led_on_off(LED_OFF);
                cyg_thread_delay(20);//0.2s
                set_lan_led_on_off(LED_ON);
                cyg_thread_delay(20);
                lan_led_blink_count --;
           }
    }
}

#if defined(__CONFIG_WAN_PORT_MASk__)
unsigned int g_pk_total;
unsigned int g_old_pk_total[5] = {0, 0, 0, 0, 0};

struct port_statistics  {
    unsigned int  rx_bytes;     
    unsigned int  rx_unipkts;       
    unsigned int  rx_mulpkts;            
    unsigned int  rx_bropkts;       
    unsigned int  rx_discard;       
    unsigned int  rx_error;          
    unsigned int  tx_bytes;     
    unsigned int  tx_unipkts;       
    unsigned int  tx_mulpkts;            
    unsigned int  tx_bropkts;       
    unsigned int  tx_discard;       
    unsigned int  tx_error;             
};

void port_mib_update(unsigned int port_id)
{
    int ret;
    struct port_statistics port_statistic;
    g_pk_total = 0;

    ret = get_port_statistic(port_id,&port_statistic);
    if (!ret) {
        g_pk_total += port_statistic.rx_unipkts;
        g_pk_total += port_statistic.rx_mulpkts;
        g_pk_total += port_statistic.rx_bropkts;
        g_pk_total += port_statistic.tx_unipkts;
        g_pk_total += port_statistic.tx_mulpkts;
        g_pk_total += port_statistic.tx_bropkts;
    }
}

void wan_led_timeout(unsigned int port_id)
{
    int blink_cunt;
    unsigned int g_wan_led_states = 0;

    if (link_stat[port_id]) {
        if (g_pk_total != g_old_pk_total[port_id]) {
            g_wan_led_states = LED_BLINK;
        } else {
            g_wan_led_states = LED_ON;
        }
    } else {
        g_wan_led_states = LED_OFF;
    }

    if (g_wan_led_states == LED_ON) {
        set_wan_led_on_off(LED_ON);
    } else if (g_wan_led_states == LED_OFF) {
        set_wan_led_on_off(LED_OFF);
    } else {
        for (blink_cunt = 4; blink_cunt >= 0; blink_cunt--) {
            set_wan_led_on_off(LED_OFF);
            cyg_thread_delay(5);
            set_wan_led_on_off(LED_ON);
            cyg_thread_delay(5);
        }
    }
    g_old_pk_total[port_id] = g_pk_total;
}
#endif

void lan_wan_led_timer()
{
    unsigned int i;
    static int set_flag = 0;

    /*如果设置了智能省电:常关/定时关闭时,不需要轮询lan_linkStatus;
     *智能省电模块 led打开：g_set_lan_wan_led= 0；
     *智能省电模块 led关闭：g_set_lan_wan_led= 1；
     *智能省电模块关不灭lan口灯，初步定位为线程间同步问题，暂时添加规避措施
    */
#ifdef  __CONFIG_ATE__
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
        set_lan_led_on_off(LED_OFF);
        set_flag = 1;
        return; 
    }
    set_flag = 0;

    port_link_update();

    for (i = 0; i < 5; i ++) {
        if ((1 << i) & __CONFIG_LAN_PORT_MASK__) {
            lan_led_timeout(i);
        }
#if defined(__CONFIG_WAN_PORT_MASk__)
        if ((1 << i) & __CONFIG_WAN_PORT_MASk__) {
            port_mib_update(i);
            wan_led_timeout(i);
        }
#endif
    }
    memcpy(old_status, link_stat, sizeof(link_stat));
}

