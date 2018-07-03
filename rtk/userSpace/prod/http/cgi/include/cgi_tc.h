#ifndef _CGI_TC_H_
#define _CGI_TC_H_

#include "arp_clients.h"

#define UP_RATE_LIMIT       301.0f       /*单位是Mbps = 128KB/s*/
#define DOWN_RATE_LIMIT     301.0f       /*单位是Mbps = 128KB/s*/

/*提供给appd,用于app设置上传下载的最大速率*/
#define UP_RATE_LIMIT_MAX       300.0f       /*单位是Mbps = 128KB/s*/
#define DOWN_RATE_LIMIT_MAX     300.0f       /*单位是Mbps = 128KB/s*/

#define TC_CLIENT_NUMBER 	255
#define STREAM_CLIENT_NUMBER TC_CLIENT_NUMBER
#define TIMES 1
#define TC_RULE_NUMBER_MAX 20
#define ETHER_ADDR_LEN 	6
#define AP_IFNAME    "wlan0"

#define WL0_MACLIST_MAX       (17*10 + 10)  /*保存wl0_maclist中的mac地址数组最大长度*/ 
#define RUB_NET_BLACKLIST_MAX 10     /* 定义一个黑名单的最大值，比如10条 */
#define TR_RULE_NUMBER_MAX    20     /*定义一个信任列表的最大值*/    
#define _FW_TRUST_MAC(i)           racat("trust_list", i)

#endif
