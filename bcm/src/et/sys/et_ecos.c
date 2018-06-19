/*
 * eCos device driver for
 * Broadcom BCM47XX 10/100 Mbps Ethernet Controller
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: et_ecos.c,v 1.8.6.1 2010-12-21 02:35:38 Exp $
 *
 */
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/pci_drv.h>
#include <net/if_types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/ethernet.h>
#include <net/netdev_init.h>

#include <osl.h>
#include <bcmendian.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmenetrxh.h>
#include <bcmenetphy.h>
#include <proto/vlan.h>
#include <pcicfg.h>
#include <et_ecos_common.h>
#include <et_ecos.h>
#include <etioctl.h>
#include <epivers.h>

#ifdef BCM_ETDBG
#define ETH_DBG(fmt, ...)				\
	do {						\
		diag_printf(fmt , ## __VA_ARGS__);	\
	} while (0)
#else
#define ETH_DBG(fmt, ...)
#endif

#ifndef ETH_CRC_LEN
#define ETH_CRC_LEN	4
#endif

#ifndef HZ
#define HZ		100
#endif

/*
 * Data Types
 */
typedef struct et_info {
	struct arpcom	sc;
#define sc_if		sc.ac_if
#define sc_enaddr	sc.ac_enaddr
	struct net_device *dev;		/* backpoint to device */

	void			*osh;		/* pointer to os handle */
	struct pci_dev	*pdev;		/* backpoint to pci_dev */
	cyg_uint32		base_addr;	/* PCI memory map base address */
	void			*regsva;
	cyg_vector_t	vector;		/* interrupt vector */
	cyg_handle_t	int_hdl;
	cyg_interrupt	int_obj;

	cyg_uint32		events;		/* events received */
	cyg_uint32		flags;

	void			*etc;		/* pointer to common os-independent data */
	cyg_uint32      unit;
	struct et_info	*next;		/* pointer to next et_info_t in chain */
	bool 			resched;	/* dpc was rescheduled */
} et_info_t;

static int et_found = 0;
static et_info_t *et_list = NULL;

#define PRIVIFFG_ACTIVE			0x01
#define PRIVIFFG_CANSEND		0x02

/*
 * Extern Function Prototypes
 */
extern void kern_schedule_dsr(void);

/*
 * Function Prototypes
 */
static void et_free(et_info_t *et);
static void et_start(void *sc);
static void et_stop(void *sc);
static int et_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data);
static int et_cansend(void *sc);
static void et_drv_send(struct ifnet *ifp);
static void et_sendup(et_info_t *et_info, struct mbuf *m);
static void et_dpc(struct ifnet *ifp);
static cyg_uint32 et_isr(cyg_vector_t vec, cyg_addrword_t data);
static void et_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data);

/* prototypes called by etc.c */
void et_init(et_info_t *et_info);
void et_reset(et_info_t *et_info);
void et_link_up(et_info_t *et_info);
void et_link_down(et_info_t *et_info);
void et_up(et_info_t *et_info);
void et_down(et_info_t *et_info, int reset);
void et_dump(et_info_t *et_info, struct bcmstrbuf *b);
void et_watchdog(struct ifnet *ifp);

/*
 * Implementation
 */
static struct pci_device_id et_id_table[] = {
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_ETHERNET << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ 0, }
};

static int et_probe(struct pci_dev *dev, const struct pci_device_id *id);

PCIDRVTAB_ENTRY(
	et_pci_driver,
	"et",
	et_id_table,
	et_probe,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL);

/* fix me */
static char et_name[] = "eth";

static int
et_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct ifnet *ifp;
	et_info_t *et;
	osl_t *osh;
	int unit = et_found;

	if (!et_chipmatch(pdev->vendor, pdev->device))
		return -ENODEV;

	ETH_DBG("et%d: find [%04x:%04x] bus %d slot %d func %d irq %d\n",
		unit, pdev->vendor, pdev->device,
		pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn), pdev->irq);

	osh = osl_attach(pdev, PCI_BUS, FALSE);
	pci_set_master(pdev);
	pci_enable_device(pdev);

	if ((et = (et_info_t*) MALLOC(osh, sizeof(et_info_t))) == NULL) {
		osl_detach(osh);
		return -ENOMEM;
	}
	bzero(et, sizeof(et_info_t));
	et->pdev = pdev;
	et->osh = osh;
	pci_set_drvdata(pdev, et);

	et->base_addr = pci_resource_start(pdev, 0);
	et->regsva = REG_MAP(et->base_addr, 0);

	if ((et->etc = et_attach(et, pdev->vendor, pdev->device, unit,
		osh, (void *)et->regsva)) == NULL)
	{
		diag_printf("%s: et_attach() failed\n", __func__);
		goto fail;
	}
	et->unit = unit;
	et->vector = pdev->irq;
	cyg_drv_interrupt_create(
		et->vector,
		0,			/* priority */
		(CYG_ADDRWORD)et,
		et_isr,
		et_dsr,
		&et->int_hdl,
		&et->int_obj);
	cyg_drv_interrupt_attach(et->int_hdl);

	/* add us to the global linked list */
	et->next = et_list;
	et_list = et;

	ifp = (struct ifnet *)et;
	et_get_hwaddr(et->etc, (char *)&et->sc_enaddr);
	ifp->if_softc = et;
	ifp->if_start = et_drv_send;
	ifp->if_ioctl = et_ioctl;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
#ifdef IFF_NOTRAILERS
	ifp->if_flags |= IFF_NOTRAILERS;
#endif
	ifp->if_name = et_name;
	ifp->if_unit = unit;
	sprintf(ifp->if_xname, "eth%d", unit);

	ether_ifattach(ifp, 0);
	ifp->if_mtu = ETHERMTU + 4; /* to accommodate the VLAN tag */
	et->dev = netdev_attach(ifp, et_dpc);
	et_found++;
	return 0;

fail:
	et_free(et);
	//osl_detach(osh);//roy remove, already done in et_free
	return (-ENODEV);
}

static void
et_free(et_info_t *et)
{
	et_info_t **prev;
	osl_t *osh;

	if (et == NULL)
		return;

	/* free common resources */
	if (et->etc) {
		et_detach(et->etc);
		et->etc = NULL;
	}

	/* remove us from the global linked list */
	for (prev = &et_list; *prev; prev = &(*prev)->next)
		if (*prev == et) {
			*prev = et->next;
			break;
		}

	osh = et->osh;
	MFREE(et->osh, et, sizeof(et_info_t));

	osl_detach(osh);
}

/*
 * interface start function
 */
static
void et_start(void *sc)
{
	et_info_t *et_info = (et_info_t *)sc;
	struct ifnet *ifp = (struct ifnet *)sc;
	struct net_device *dev = et_info->dev;
	int s;

	ETH_DBG("%s start\n", ifp->if_xname);
	if (!(et_info->flags & PRIVIFFG_ACTIVE))
	{
		et_info->flags |= PRIVIFFG_ACTIVE;
		et_promisc(et_info->etc, TRUE);
		/* Enable allmulti when promisc enabled */
		et_allmulti(et_info->etc, TRUE);
		s = splimp();
		et_up(et_info);
		splx(s);
	}

	ifp->if_flags |= IFF_RUNNING;
	ifp->if_flags &= ~IFF_OACTIVE;
	dev->flags |= NETDEV_FLAG_RUNNING;

	et_drv_send(ifp);

}

/*
 * interface down function
 */
static void
et_stop(void *sc)
{
	et_info_t *et_info = (et_info_t *)sc;
#ifdef BCM_ETDBG
	struct ifnet *ifp = (struct ifnet *)sc;
#endif
	struct net_device *dev = et_info->dev;
	int s;

	ETH_DBG("%s stop\n", ifp->if_xname);

	if (et_info->flags & PRIVIFFG_ACTIVE)
	{
		/* Disable allmulti when promisc disabled */
		et_allmulti(et_info->etc, FALSE);
		et_promisc(et_info->etc, FALSE);
		s = splimp();
		et_down(et_info, 1);
		splx(s);
		et_info->flags &= ~PRIVIFFG_ACTIVE;
	}

	dev->flags &= ~NETDEV_FLAG_RUNNING;
}

static int et_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	et_info_t *et_info = (et_info_t *)ifp;
	struct ifaddr *ifa = (struct ifaddr *)data;
	struct ifreq *ifr = (struct ifreq *)data;
	int s, error = 0;

	s = splnet();

	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			et_start(et_info);
			arp_ifinit(&et_info->sc, ifa);
			break;
#endif
		default:
			et_start(et_info);
			break;
		}
		break;

	case SIOCGIFHWADDR:
		ifr->ifr_hwaddr.sa_family = AF_LINK;
		bcopy(&et_info->sc_enaddr, &ifr->ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
		break;

	case SIOCSIFHWADDR:
		bcopy(&ifr->ifr_hwaddr.sa_data, &et_info->sc_enaddr, ETHER_ADDR_LEN);
		et_set_hwaddr(et_info->etc, (char *)&ifr->ifr_hwaddr.sa_data);
		break;

	case SIOCSIFFLAGS:
		if ((ifp->if_flags & IFF_UP) == 0 && (ifp->if_flags & IFF_RUNNING) != 0) {
			/*
			 * If interface is marked down and it is running, then
			 * stop it.
			 */
			et_stop(et_info);
			ifp->if_flags &= ~IFF_RUNNING;
		}
		else if (((ifp->if_flags & IFF_UP) != 0) && (ifp->if_flags & IFF_RUNNING) == 0) {
			/*
			 * If interface is marked up and it is stopped, then
			 * start it.
			 */

			et_start(et_info);
		}
		else {
			et_stop(et_info);

			et_start(et_info);
		}
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		/*  Accept all multicast frame  */
		et_allmulti(et_info->etc, TRUE);

		et_init(et_info);
		break;
#ifdef	SIOCGIFLINK
	case SIOCGIFLINK:
	{
		struct phystatus *ps = (struct phystatus *)ifr->ifr_data;

		if (et_get_linkstate(et_info->etc, ps->ps_port))
			ps->ps_link = 1;
		else
			ps->ps_link = 0;

		break;
	}
#endif
#ifdef SIOCGIFBAUD
	case SIOCGIFBAUD:
	{
		struct phystatus *ps = (struct phystatus *)ifr->ifr_data;

		ps->ps_baud = et_get_linkspeed(et_info->etc, ps->ps_port);

		break;
	}
#endif
#ifdef SIOCGIFSPEED
	case SIOCGIFSPEED:
	{
		struct phystatus *ps = (struct phystatus *)ifr->ifr_data;
		int speed = et_get_linkspeed(et_info->etc, ps->ps_port);
		int duplex = et_get_duplex(et_info->etc, ps->ps_port);

		if (speed == 10 && duplex == 0)
			ps->ps_speed = PORT_SPEED_10HALF;
		else if (speed == 10 && duplex == 1)
			ps->ps_speed = PORT_SPEED_10FULL;
		else if (speed == 100 && duplex == 0)
			ps->ps_speed = PORT_SPEED_100HALF;
		else
			ps->ps_speed = PORT_SPEED_100FULL;
		break;
	}
#endif /* SIOCGIFSPEED */
#ifdef SIOCSIFSPEED
	case SIOCSIFSPEED:
	{
		struct phystatus *ps = (struct phystatus *)ifr->ifr_data;

		if (et_set_speed(et_info->etc, ps->ps_port, ps->ps_speed) != 0)
			error = EINVAL;
		break;
	}
#endif

#ifdef SIOCSETQOS
	case SIOCSETQOS:
	{
		unsigned int on = *(unsigned int *)ifr->ifr_data;

		diag_printf("%s Ethernet QoS\n", on? "Enable":"Disable");
		error = et_etc_ioctl(et_info->etc,
			SIOCSETCQOS - SIOCSETCUP, ifr->ifr_data) ? EINVAL : 0;
		break;
	}
#endif

	case SIOCETHTOOL:
		error = -EINVAL;
		break;

	case SIOCSIFMTU:
		ifp->if_mtu = ifr->ifr_mtu;
		break;

	default:
		error = -EINVAL;

		if (cmd >= SIOCSETCUP) {
			error = et_etc_ioctl(et_info->etc,
				cmd - SIOCSETCUP, ifr->ifr_data) ? EINVAL : 0;
			break;
		}
	}

	splx(s);

	return (error);
}

static int
et_cansend(void *sc)
{
	et_info_t *et_info = (et_info_t *)sc;

	if ((et_info->flags & PRIVIFFG_CANSEND) && (et_txdesc_num(et_info->etc) > 4))
		return 1;
	else
		return 0;
}

static void
et_drv_send(struct ifnet *ifp)
{
	et_info_t *et_info;
	void *etc;
	int len, total_len;
	struct mbuf *m0, *m;
	int cansend;
	int s,sd=0;
	if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
		return;
	}

	et_info = (et_info_t *)ifp;
	etc = et_info->etc;

	s = splimp();
#define	BCM4718GMAC_LIMIT	18
	while ((cansend = et_cansend(et_info)) > 0)
	{
		IF_DEQUEUE(&ifp->if_snd, m0);
		if (m0 == 0)
			break;
		if ((m0->m_flags & M_PKTHDR) == 0)
			panic("et_drv_send: no header mbuf");

		if (m0->m_len < BCM4718GMAC_LIMIT) {
			m0 = m_pulldown(m0, 0, BCM4718GMAC_LIMIT, NULL);
		}
		total_len = 0;
		for (m = m0; m; m = m->m_next) {
			len = m->m_len;
			total_len += len;
		}

		/*to solve the 802.1x packets with vlan tag, add by cdy*/
		if ((0x88 == (uchar)m0->m_data[16] )&& (0x8e == (uchar)m0->m_data[17])){
			for (sd = 12; sd < m0->m_len; sd++ ){
				m0->m_data[sd] = m0->m_data[sd+4];
			}
			m0->m_len -= 4;
			total_len -= 4;
		}

		et_xmit(etc, (void*)m0, total_len);
		ifp->if_opackets++;
	}
	splx(s);
}

static void
et_sendup(et_info_t *et_info, struct mbuf *m)
{
	struct ifnet *ifp;
	void *etc = et_info->etc;
	bcmenetrxh_t *rxh;
	uint16 flags;
	struct ether_header* eh;

	rxh = mtod(m, bcmenetrxh_t *);

	/* strip off rxhdr */
	m_adj(m, et_rx_hwoffset());
	et_recv_update(etc, m->m_len);

	/* strip off crc32 */
	m_adj(m, -ETH_CRC_LEN);

	/* get the error flags */
	flags = et_etc_rxh_flags(etc, rxh);

	/* check for reported frame errors */
	if (flags)
		goto err;
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.priority = 0;
	if (et_qos_pkt(etc)) {
		pktsetprio(m, FALSE);
	}
	ifp = (struct ifnet *)et_info;
	if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
		/* Interface not up, ignore this request */
		m_freem(m);
		return;
	}
	m->m_pkthdr.rcvif = ifp;

	eh = (struct ether_header*)m->m_data;

	if (!(ntohs(eh->ether_type) == ETHERTYPE_VLAN))
		m_adj(m, sizeof(struct ether_header));
	/* Push data into protocol stacks */
	ether_input(ifp, eh, m);
	ifp->if_ipackets++;
	return;
err:
	et_show_errors(etc,
		(struct ether_addr*)((struct ether_header*)m->m_data)->ether_shost, flags);
	m_freem(m);

	return;
}

static void et_dpc(struct ifnet *ifp)
{
	et_info_t *et_info = (et_info_t *)ifp;
	void *etc = et_info->etc;
	struct mbuf *m;
	int s;

	s = splimp();

	if (!et_chip_up(et_info->etc))
		goto done;

	/* get interrupt condition bits again when dpc was rescheduled */
	if (et_info->resched) {
		et_info->events  = et_int_events(etc, FALSE);
		et_info->resched = FALSE;
	}

	if (et_rx_event(et_info->events))
	{
		uint processed = 0;
		while ((m = (struct mbuf *)et_recv_pkt(etc)))
		{
			et_sendup(et_info, m);
			/* more frames, need to reschedule et_dpc() */
			if (++processed >= RXBND) {
				et_info->resched = TRUE;
				break;
			}
		}
		/* post more rx bufs */
		et_rxfill(etc);
	}

	if (et_tx_event(et_info->events))
	{
		et_txreclaim(etc);
	}

	/* Try to post rx bufs to prevent the rx dma channel from being stuck. */
	et_rxfill(etc);

	if (et_error_event(et_info->events))
	{
		if (et_chip_errors(etc))
			et_init(et_info);
	}

	et_info->events = 0;

	/* something may bring the driver down */
	if (!et_chip_up(et_info->etc)) {
		et_info->resched = FALSE;
		goto done;
	}

	/* there may be frames left, reschedule et_dpc() */
	if (et_info->resched) {
		struct net_device *dev = et_info->dev;
		dev->flags |= NETDEV_FLAG_INTPENDING;
		kern_schedule_dsr();
	}
	/* enable interrupts */
	else
		et_enable_int(etc);

done:
	splx(s);
	return;
}

static cyg_uint32 et_isr(cyg_vector_t vec, cyg_addrword_t data)
{
	et_info_t *et_info = (et_info_t *)data;
	void *etc = et_info->etc;
	cyg_uint32 events;

	if (!et_chip_up(etc))
		return 0;

	events = et_int_events(etc, TRUE);
	if (!(et_new_event(events)))
		return 0;

	et_disable_int(etc); /* disable interrupts */
	cyg_drv_interrupt_acknowledge(vec);
	et_info->events = events;

	return CYG_ISR_CALL_DSR;
}

static void et_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
	et_info_t *et_info = (et_info_t *)data;
	struct net_device *dev = et_info->dev;

	if (!et_chip_up(et_info->etc))
		return;

	if (dev) {
		dev->flags |= NETDEV_FLAG_INTPENDING;
		kern_schedule_dsr();
	}
}

/* prototypes called by etc.c */
void
et_init(et_info_t *et_info)
{
	et_reset(et_info);
	et_chip_init(et_info->etc);

	/* dpc will not be rescheduled */
	et_info->resched = 0;
}

void
et_reset(et_info_t *et_info)
{
	et_chip_reset(et_info->etc);
	et_info->events = 0;
}

void
et_link_up(et_info_t *et_info)
{
}

void
et_link_down(et_info_t *et_info)
{
}

void
et_up(et_info_t *et_info)
{
	void *etc = et_info->etc;

	if (et_chip_up(etc))
		return;

	et_chip_open(etc);
	et_info->flags |= PRIVIFFG_CANSEND;
	cyg_drv_interrupt_acknowledge(et_info->vector);
	cyg_drv_interrupt_unmask(et_info->vector);
}

void
et_down(et_info_t *et_info, int reset)
{
	cyg_drv_interrupt_mask(et_info->vector);
	et_info->flags &= ~PRIVIFFG_CANSEND;
	et_chip_close(et_info->etc, reset);
}

void
et_dump(et_info_t *et, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "et%d: %s %s version %s\n", et_etc_unit(et->etc),
		__DATE__, __TIME__, EPI_VERSION_STR);
#ifdef BCMDBG
	et_dbg_dump(et->etc, b);
#endif
}

/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void*
et_phyfind(et_info_t *et, uint coreunit)
{
	et_info_t *tmp;
	uint bus, slot;

	bus = et->pdev->bus->number;
	slot = PCI_SLOT(et->pdev->devfn);

	/* walk the list et's */
	for (tmp = et_list; tmp; tmp = tmp->next) {
		if (et->etc == NULL)
			continue;
		if (tmp->pdev == NULL)
			continue;
		if (tmp->pdev->bus->number != bus)
			continue;
		if (et_nicmode(tmp->etc))
			if (PCI_SLOT(tmp->pdev->devfn) != slot)
				continue;
		if (et_core_unit(tmp->etc) != coreunit)
			continue;
		break;
	}
	return (tmp);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et_info, uint phyaddr, uint reg)
{
	uint16 val;
	int s;

	s = splimp();
	val = et_chip_phyrd(et_info->etc, phyaddr, reg);
	splx(s);

	return (val);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et_info, uint phyaddr, uint reg, uint16 val)
{
	int s;

	s = splimp();
	et_chip_phywr(et_info->etc, phyaddr, reg, val);
	splx(s);
}

void
et_intrson(et_info_t *et_info)
{
	et_enable_int(et_info->etc);
}

void
et_watchdog(struct ifnet *ifp)
{
	et_info_t *et_info = (et_info_t *)ifp;

	et_chip_watchdog(et_info->etc);
	ifp->if_timer = 1;
}
