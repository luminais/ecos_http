#ifndef _APCLIENT_DHCPC_H_
#define _APCLIENT_DHCPC_H_
#include "pi_common.h"

#define CONNECTED_COUNT_MAX			5 
#define DISCONNECTED_COUNT_MAX 		5
#define APCLINET_DHCPC_SLEEP		(3 * 100)

typedef struct apclient_dhcpc_info_struct
{
	unsigned int enable;
	char lan_ifnames[MAX_IP_LEN];
	char ipaddr[MAX_IP_LEN];
	char mask[MAX_IP_LEN];
	WANSTATUS now_status;
	WANSTATUS before_status;
	unsigned int connected_count;
	unsigned int disconnected_count;
} APCLIENT_DHCPC_INFO_STRUCT,*P_APCLIENT_DHCPC_INFO_STRUCT;
#endif
