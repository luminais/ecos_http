#ifndef _IF_STATUS_H_
#define _IF_STATUS_H_
 
#define IF_DOWN 		0
#define IF_UP   		1
#define IF_STATUS_CHANGE	1
#define IF_STATUS_CLEAR		0

/*
 * eth ifterface(0 ~ 7) 
 * wlan0 ifterface(8 ~ 20)
 * wlan1 ifterface(21 ~ 32)
 */
#define ETH_PORT(n)		n

#define WAN_PORT		4 
#define WLAN0_PORT      8
#define WLAN0_VXD_PORT  9
#define WLAN1_PORT      20
#define WLAN1_VXD_PORT  21

int get_if_status(unsigned int IfType);
int set_if_status(unsigned int IfType,int value);
int get_if_change_status(unsigned int IfType);
int set_if_change_status(unsigned int IfType,int value);


#endif
