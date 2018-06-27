/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: tai_main.h
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.1.27
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.27   1.0          new
************************************************************/
#include "alinkgw_api.h"
#include "tai.h"
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <netinet/if_ether.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>


#define STACK_SIZE 65535 

static cyg_handle_t tai_daemon_handle;
static char tai_daemon_stack[STACK_SIZE];
static cyg_thread tai_daemon_thread;

extern int get_ali_sec_info(char *buff, unsigned int buff_sz);

extern int set_ali_sec_info(const char *json_in) ;

extern int get_wlan_switch_state(char *buff, unsigned int buff_sz) ;

extern int set_wlan_switch_state(const char *json_in) ;

extern int report_wlanswitch_state(void) ;

extern int check_wlanswitch_state(void) ;

extern int authDevice(const char *json_in, char *buff, unsigned int buf_sz) ;

extern int save_kvp_value(const char* name , const char* value) ;

extern int get_kvp_value(const char* name , char* value , unsigned int buf_sz) ;

extern void connection_status_update(ALINKGW_CONN_STATUS_E status) ;

extern int get_wlan_switch_scheduler(char *buff, unsigned int buff_len);

extern int set_wlan_switch_scheduler(const char *json_in);

extern int get_probe_info(char *buf, unsigned int buff_len);

extern int set_probe_info(const char *jsonin);

extern int check_wlanswitchscheduler_state();
  
extern int report_wlanswitchscheduler_state();

extern int set_probe_num(const char *jsonin);

extern int get_probe_num(char *buf, unsigned int buff_len);

extern void check_alilink_probe_info();

extern int get_tpsk(char *buff, unsigned int buff_len);

extern int set_tpsk(const char *json_in);

extern int get_tpsk_list(char *buff, unsigned int buff_len);

extern int set_tpsk_list(const char *json_in);

extern int load_tpsk_config();

extern int get_router_common_attribute(struct DEV_ATTRIBUTE* r_attribute);

extern int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);

extern int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);

extern int set_probed_switch_state(const char *json);

extern int get_probed_switch_state(char *buf, unsigned int buff_len);

extern int set_attack_switch_state(const char *json);

extern int get_attack_switch_state(char *buf, unsigned int buff_len);

extern int ChangePassword(const char *json_in, char *buff, unsigned int buf_sz);

extern int get_wandl_speed(char *buf, unsigned int buff_len);

extern int get_wanul_speed(char *buf, unsigned int buff_len);

extern void wan_speed_report(void);

extern int set_wan_speed_detect(const char *json_in, char *buf, unsigned int sz);

extern int get_wlansetting24g(char *buff, unsigned int buff_sz) ;

extern int set_wlansetting24g(const char *json_in);

extern int get_wlansecurity24g(char *buff, unsigned int buff_sz) ;

extern int set_wlansecurity24g(const char *json_in );

extern int get_wlan_channel_24g(char *buff, unsigned int buff_sz);

extern int get_wlanpamode(char *buff, unsigned int buff_sz);

extern int set_wlanpamode(const char *json_in ) ;

extern int set_qos_speedup(const char *json_in) ;

extern int get_qos_speedup(char *buff, unsigned int buff_sz) ;

extern int subdevice_get_upspeed_value(const char *subdev_mac , char *buf,  unsigned int buf_sz);

extern int subdevice_get_downspeed_value(const char *subdev_mac , char *buf,  unsigned int buf_sz);

extern int rt_srv_refineWlanChannel(const char *json_in, char *buff, unsigned int buff_len);

extern void init_ip_list();