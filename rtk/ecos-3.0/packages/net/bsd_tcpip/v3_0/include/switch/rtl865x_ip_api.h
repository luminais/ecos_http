#ifndef RTL865X_IP_API_H
#define RTL865X_IP_API_H

#define IP_TYPE_NAPT            0x00
#define IP_TYPE_NAT             0x01
#define IP_TYPE_LOCALSERVER     0x02
#define IP_TYPE_RESERVED        0x03

int rtl865x_initIpTable(void);
int rtl865x_reinitIpTable(void);
int rtl865x_addIp(unsigned long intIp, unsigned long extIp, unsigned int ip_type);
int rtl865x_delIp(unsigned long extIp);
#endif
