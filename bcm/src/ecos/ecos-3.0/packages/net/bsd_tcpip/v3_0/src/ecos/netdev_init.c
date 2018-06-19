/*
 * $ Copyright Open Broadcom Corporation $
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <net/netisr.h>
#include <cyg/infra/cyg_type.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/netdev_init.h>

#include <cyg/io/eth/pci_drv.h>
CYG_HAL_TABLE_BEGIN( __PCIDRVTAB__, pcidrv );
CYG_HAL_TABLE_END( __PCIDRVTAB_END__, pcidrv );

#undef malloc
#undef free

void *malloc(size_t size);
void free(void *ptr);

extern void bridgeintr(void);

#define MAX_NETDEV_COUNT 6

struct net_device *netdev_tab[MAX_NETDEV_COUNT] = { 0 };
static int netdev_count = 0;

struct net_device *netdev_attach(struct ifnet *ifp, void (*func)(struct ifnet *ifp))
{
	struct net_device *dev;
	
	if (netdev_count >= MAX_NETDEV_COUNT)
		goto bad;
	
	if (ifp == NULL)
		goto bad;
	
	dev = (struct net_device *)malloc(sizeof(struct net_device));

	if (!dev)
		goto bad;
	
	dev->ifp = ifp;
	dev->flags = 0;
	dev->int_func = func;
	netdev_tab[netdev_count] = dev;
	netdev_count++;
	
	return dev;
	
bad:
	return NULL;
}

void netdev_run_intfuncs(void)
{
	struct net_device *dev;
	int i;

	for (i=0; i<MAX_NETDEV_COUNT; i++) {
		if ((dev=netdev_tab[i])) {
			if ((dev->flags & NETDEV_FLAG_INTPENDING) && dev->int_func){
				dev->flags &= ~NETDEV_FLAG_INTPENDING;
				(*dev->int_func)(dev->ifp);
			}
		}
	}
}

void netdev_tickle_devices(void)
{
	struct net_device *dev;
	int i;
	for (i=0; i<MAX_NETDEV_COUNT; i++) {
		if ((dev=netdev_tab[i])) {
			if (dev->flags & NETDEV_FLAG_RUNNING) {
				struct ifnet *ifp = dev->ifp;
				(*ifp->if_start)(ifp);
			}
		}
	}
}
extern void bridgeintr(void);

void bcm_netdev_init(void)
{
	pci_driver_t *pdrv;

	/* register network protocol handler */
	register_netisr(NETISR_DSR, netdev_run_intfuncs);
	register_netisr(NETISR_DEVTICK, netdev_tickle_devices);
	register_netisr(NETISR_BRIDGE, bridgeintr);

	/* register device driver to PCI device list */
	for (pdrv = &__PCIDRVTAB__[0]; pdrv != &__PCIDRVTAB_END__; pdrv ++)
		pci_register_driver(pdrv);
}

void
kern_schedule_dsr(void)
{
	schednetisr(NETISR_DSR);
}
