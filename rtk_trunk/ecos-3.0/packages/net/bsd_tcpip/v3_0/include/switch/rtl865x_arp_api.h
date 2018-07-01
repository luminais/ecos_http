#ifndef RTL865X_ARP_API_H
#define RTL865X_ARP_API_H

#define RTL865X_ARPTBL_SIZE 512

/*for driver initialization*/
int rtl865x_arp_init(void);
int rtl865x_arp_reinit(void);

/*for linux protocol stack sync*/
int rtl865x_addArp(unsigned long ip, unsigned char * mac);
int rtl865x_delArp(unsigned long ip);
unsigned int rtl865x_arpSync( unsigned long ip, unsigned int refresh );
int rtl865x_updateFdbByArp(unsigned long ip);

#endif
