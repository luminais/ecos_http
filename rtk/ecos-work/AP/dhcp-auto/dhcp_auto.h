#ifndef DHCP_AUTO_H
#define DHCP_AUTO_H
#define DHCPAUTO_DEBUG 1
#define DHCPAUTO_CHECK_INTERVAL 3 //second
#define HOLD_COUNT_MAX 20
#define OFF_LINK_COUNT_MAX 2
#define WAIT_IP_COUNT_MAX 15
typedef enum DHCP_AUTO_STATUS_
{
	DHCP_AUTO_STATUS_OFFLINK,
	DHCP_AUTO_STATUS_HOLD_REQIP,
	DHCP_AUTO_STATUS_REQIP,
	DHCP_AUTO_STATUS_HOLD_GETIP,
	DHCP_AUTO_STATUS_GETIP	
} DHCP_AUTO_STATUS;

#ifndef DHCPAUTO_DEBUG
#define dhcpauto_debug_printf(n, args...) do{}while(0) 
#else
#define dhcpauto_debug_printf diag_printf
#endif
#endif
