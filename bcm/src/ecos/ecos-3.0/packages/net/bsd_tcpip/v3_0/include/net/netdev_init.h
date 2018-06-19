/*
 * $ Copyright Open Broadcom Corporation $
 */

#ifndef _NETDEV_INIT_H_
#define _NETDEV_INIT_H_

#include <net/if_var.h>

struct net_device {
	struct ifnet *ifp;
	unsigned int flags;
	void (*int_func)(struct ifnet *ifp);
};

#define NETDEV_FLAG_RUNNING	0x1
#define NETDEV_FLAG_INTPENDING	0x2

struct net_device *netdev_attach(struct ifnet *ifp, void (*func)(struct ifnet *ifp));

void bcm_netdev_init(void);

void kern_schedule_dsr(void);

#endif /* #define _NETDEV_INIT_H_ */
