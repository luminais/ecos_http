#ifndef RTL865X_ARP_H
#define RTL865X_ARP_H

#include <switch/rtl865x_arp_api.h>

typedef struct rtl865x_arpMapping_entry_s
{
        unsigned long ip;
        ether_addr_t mac;
}rtl865x_arpMapping_entry_t;


struct rtl865x_arp_table {
        unsigned char                                           allocBitmap[64];
        rtl865x_arpMapping_entry_t      mappings[512];
};

/*for routing module usage*/
int32 rtl865x_arp_tbl_alloc(rtl865x_route_t *route);
int32 rtl865x_arp_tbl_free(rtl865x_route_t *route);
int32 rtl865x_getArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping);
#endif

