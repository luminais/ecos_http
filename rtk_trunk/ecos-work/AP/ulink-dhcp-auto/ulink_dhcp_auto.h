#ifndef ULINK_DHCP_AUTO_H
#define ULINK_DHCP_AUTO_H
//#define ULINK_DHCPAUTO_DEBUG 1
#define ULINK_DHCPAUTO_CHECK_INTERVAL 1 //second
typedef enum ULINK_DHCP_AUTO_STATUS_
{
	ULINK_DHCP_AUTO_STATUS_OFFLINK,
	ULINK_DHCP_AUTO_STATUS_REQIP,
	ULINK_DHCP_AUTO_STATUS_GETIP
} ULINK_DHCP_AUTO_STATUS;

#ifndef ULINK_DHCPAUTO_DEBUG
#define ulink_dhcpauto_debug_printf(n, args...) do{}while(0) 
#else
#define ulink_dhcpauto_debug_printf diag_printf
#endif
#endif
