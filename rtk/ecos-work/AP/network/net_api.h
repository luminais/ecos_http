#ifndef NET_API_H
#define NET_API_H

int is_interface_up(const char *ifname);
int interface_up(const char *intf);
int interface_down(const char *intf);
int interface_config(const char *intf, char *addr, char *netmask);
int shutdown_all_interfaces(void);
void show_network(void);
void show_phy_stats(void);
int route_add(int argc, char **argv);

#endif /* NET_API_H */
