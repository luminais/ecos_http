/*************************************************************************
	> Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
	> File Name: tenda_arp.h
	> Description: 
	> Author: zhuhuan
	> Mail: zhuhuan_IT@outlook.com
	> Version: 1.0
	> Created Time: Sun 24 Jan 2016 08:57:37 PM CST
	> Function List: 

	> History:
    <Author>      <Time>      <Version>      <Desc>
    
 ************************************************************************/

#ifndef _TENDA_ARP_H_
#define _TENDA_ARP_H_

#define TENDA_ARP_MAX_NUM (255)
#define TENDA_ARP_SIZE(array) (sizeof(array)/sizeof(array[0]))
#define ETHER_ADDR_LEN (6)
#define ADD_ITEM (1)
#define DELETE_ITEM (2)
#define MODIFY_ITEM (3)

struct detec
{
	struct detec *next;
    char auth;
    unsigned char mac[ETHER_ADDR_LEN];
    in_addr_t ip;
	char hostname[64];
	int flag;
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	u_long rmx_expire;//老化时间
	time_t time; //sta第一次接入路由器的时间
	time_t interval_time;//sntp更新前的时间间隔
	time_t update_time; //最近一次更新时间
#endif
};

/* Function declaration */
int tenda_arp_is_wireless_client(unsigned char *mac);
int tenda_arp_get_auth(unsigned char* mac);
void tenda_arp_clear_auth_all(void);
unsigned char *tenda_arp_ip_to_mac(in_addr_t ip);
in_addr_t tenda_arp_mac_to_ip(char *mac);
char *tenda_arp_mac_to_hostname(unsigned char *mac);
int tenda_arp_mac_to_flag(unsigned char *mac);
int tenda_arp_ip_to_flag(in_addr_t ip);
unsigned int tenda_arp_get_online_client_num(void);
int tenda_arp_is_online(in_addr_t ip);
int tenda_arp_is_multicast(unsigned char *mac);
inline int tenda_arp_isnot_lanhost(in_addr_t ip);
int tenda_arp_update_node(unsigned char *mac,in_addr_t ip,int action, u_long rmx_expire);
char *tenda_arp_inet_ntoa(in_addr_t ina);
void tenda_arp_get_all(struct detec *buf);
int tenda_arp_update_all();
void tenda_arp_show(int argc, char* argv[]);
void tenda_arp_set_interval_time();
void tenda_arp_update_time();
void tenda_arp_delete_expired_list(void);
void tenda_arp_init(void);
/* End declaration */ 

#endif
