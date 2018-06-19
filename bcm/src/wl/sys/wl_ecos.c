/*
 * eCos-specific portion of Broadcom 802.11
 * Networking Adapter Device Driver.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wl_ecos.c,v 1.9.4.4 2011-01-20 09:52:35 Exp $
 */

#include <stdio.h>
#include <typedefs.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/eth/pci_drv.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/mbuf.h>

#include <epivers.h>
#include <proto/ethernet.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/netdev_init.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <bcmdevs.h>
#include <wl_dbg.h>

#define __INCif_etherh
#include <wlioctl.h>
#undef __INCif_etherh

#include <net/if.h>

#include <sys/ioctl.h>
#include <wl_ecos_comm.h>
#include <wlc_pub.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <ethtool.h>
#include <proto/802.1d.h>

#ifdef BCMDBG
#define DEBUG
#endif

#define MAX_TX_DESCRIPTORS		128
#define DATAHIWAT				50
#define ETH_STATS_INIT(p)
#define CMN_IOCTL_END			(CMN_IOCTL_OFF+WLC_LAST)

#define WL_IFTYPE_BSS	1 /* iftype subunit for BSS */
#define WL_IFTYPE_WDS	2 /* iftype subunit for WDS */

#define PRIVIFFG_ACTIVE			0x01
#define PRIVIFFG_CANSEND		0x02

extern char *ether_sprintf(const u_char *ap);


/* For each interface */
typedef struct wl_if {
	struct arpcom	sc;
#define sc_if		sc.ac_if
#define sc_enaddr	sc.ac_enaddr

	struct wl_if	*next;
	struct wl_info	*wl;		/* back pointer to main wl_info_t */
	struct net_device	*dev;		/* virtual netdevice */
	int		type;     	/* interface type: WDS, BSS */
	void		*wlcif;		/* wlc interface handle */
	struct ether_addr	remote;		/* remote WDS partner */
	uint		txqstopped;	/* tx flow control */
	uint		unit;		/* WDS unit */
	char		name[IFNAMSIZ];
};

typedef struct wl_info {
	wlc_pub_t	*pub;		/* pointer to public wlc state */
	wlc_info_t	*wlc;		/* pointer to private common os-independent data */
	wl_if_t         	wlif;		/* list of all interfaces */
	void		*osh;		/* pointer to os handle */
	struct pci_device	*pdev;		/* pointer to pci device structure */
	uint		bustype;
	cyg_uint32	base_addr;	/* PCI memory map base */
	void		*regsva;
	cyg_vector_t	vector;		/* interrupt vector */
	cyg_handle_t  	interrupt_handle;
	cyg_interrupt	interrupt_object;
	cyg_uint32	flags;

	struct wl_info  	*next;		/* pointer to next wl_info_t in chain */
	uint 		callbacks;	/* # outstanding callback functions */
	cyg_mutex_t	lock;
	bool		resched;	/* dpc needs to be and is rescheduled */
} wl_info_t;

typedef struct wl_timer {
	struct wl_info	*wl;
	void		(*fn)(void *);
	void		*arg;
	uint		tick;
	bool		periodic;
	bool		set;
	unsigned int	timer;
#ifdef BCMDBG
	char*		name;		/* Desription of the timer */
#endif
} wl_timer_t;

static wl_info_t *wl_list = NULL;
static int wl_found = 0;

#define WL_LOCK(wl)	cyg_scheduler_lock()
#define WL_UNLOCK(wl)	cyg_scheduler_unlock()

#define WL_INFO(ifp)	((wl_if_t *)(ifp)->if_softc)->wl
#define WLIF_IFP(ifp)	((wl_if_t *)(ifp)->if_softc)
#define IFP_WLIF(wlif)	((struct ifnet *)wlif)

/* ====================================================================== */
/*  Extern function */
extern int br_add_if(int index, char *ifname);
extern int br_del_if(int index, char *ifname);
extern void kern_schedule_dsr(void);

/* ====================================================================== */
/*  Interface function for uppler layer  				*/
static int  wl_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data);
static int  wl_cansend(wl_info_t *wl);

/* ====================================================================== */
/*  Interface initialize function					*/
static cyg_uint32 wl_isr(cyg_vector_t vector, cyg_addrword_t data);
static void wl_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static void wl_mic_error(wl_info_t *wl, struct ether_addr *ea, bool group, bool flush_txq);
void wl_intrson(wl_info_t *wl);
char *wl_ifname(struct wl_info *wl, struct wl_if *wlif);

#define WL_DEV_NAME	"wl"
#define WDS_DEV_NAME	"wds"

/* eCos Entry points	*/
/* ====================================================================== */

static struct pci_device_id wl_id_table[] = {
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_OTHER << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ 0, }
};

static int wl_pci_probe(struct pci_dev *dev, const struct pci_device_id *id);

PCIDRVTAB_ENTRY(
	wl_pci_driver,
	"wl",
	wl_id_table,
	wl_pci_probe,
	NULL /* wl_remove */,
	NULL,
	NULL /* wl_suspend */,
	NULL /* wl_resume */,
	NULL);

static void
wl_drv_send(struct ifnet *ifp)
{
	wl_info_t *wl = WL_INFO(ifp);
	wl_if_t *wlif = WLIF_IFP(ifp);
	int len, total_len;
	struct mbuf *m0, *m;
	struct ether_header *eh;
	void *osh;
	void *pkt;
	int cansend;

	if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING)
		return;
		
	osh = wl_ecos_comm_osh(wl->pub);
		
	WL_LOCK(wl);
	while ((cansend = wl_cansend(wl)) > 0) {
		IF_DEQUEUE(&ifp->if_snd, m0);
		if (m0 == 0)
			break;

		if (!wl_ecos_comm_up(wl->pub)) {
			PKTFREE(osh, m0, true);
			goto done;
		}
		if ((m0->m_flags & M_PKTHDR) == 0)
			panic("wl_drv_send: no header mbuf");

		total_len = 0;
		for (m = m0; m; m = m->m_next) {
			len = m->m_len;
			total_len += len;
		}

		/* Convert the packet. Mainly attach a pkttag */
		if ((pkt = PKTFRMNATIVE(osh, m0)) == NULL) {
			wl_ecos_comm_cntincr_cnt_txnobuf(wl->pub);
			PKTFREE(osh, m0, true);
			goto done;
		}

		/* Fix the priority if WME is enabled */
		if (WME_ENAB(wl->pub) && (PKTPRIO(pkt) == 0))
			pktsetprio(pkt, FALSE);
	



		/*
		  * WAR, refill eth source mac when packet
		  * is EAPOL and rcvif is NULL
		  */
		eh = (struct ether_header *) PKTDATA(osh, m0);
		if (eh && !m0->m_pkthdr.rcvif &&
			ntoh16(eh->ether_type) == ETHER_TYPE_802_1X) {
			bcopy((char*)&wlif->sc_enaddr, (char*)&eh->ether_shost, ETHER_ADDR_LEN);
		}

		
#define TD_ICMP_PRIO 1
#if TD_ICMP_PRIO
			if(eh){
				if(ntoh16(eh->ether_type) == ETHER_TYPE_IP )
				{
					uint8 *iph;
					if (PKTLEN(osh, m0) > ETHER_HDR_LEN + 9)////IPV4_PROT_OFFSET =9
					{	
						iph = (uint8 *)eh + ETHER_HDR_LEN;
						if (iph[9]  == 1)  //IP_PROT_ICMP=0x1
						{	
							pktsetprio(pkt, PRIO_8021D_VO);
						}
					}
				}
			}
#endif
			
		wl_ecos_comm_wlc_sendpkt(wl->wlc, m0, wlif->wlcif);

		ifp->if_opackets++;
#ifdef TENDA_WLAN_DBG
        /* add by jack,2015-10-9,更新驱动发包统计 */
        wl_ecos_comm_update_if_opackets(wl->wlc,ifp->if_opackets);
        /* end by jack */
#endif
	}
done:
	WL_UNLOCK(wl);
}

static void wl_dpc(struct ifnet *ifp)
{
	wl_info_t *wl = WL_INFO(ifp);

	WL_LOCK(wl);
	if (!wl_ecos_comm_up(wl->pub))
		goto done;

	if (wl->resched)
		wl_ecos_comm_wlc_intrsupd(wl->wlc);

	wl->resched = wl_ecos_comm_wlc_dpc(wl->wlc, true);

	if (!wl_ecos_comm_up(wl->pub))
		goto done;

	/* re-enable interrupts */
	if (wl->resched) {
		struct net_device *dev = (wl->wlif.dev);

		dev->flags |= NETDEV_FLAG_INTPENDING;
		kern_schedule_dsr();
	}
	else
		wl_intrson(wl);

done:
	WL_UNLOCK(wl);
	return;
}

void
wl_free(wl_info_t *wl)
{
	osl_t *osh;

	WL_TRACE(("wl: wl_free\n"));

	/* free common resources */
	if (wl->wlc) {
		wl_ecos_comm_wlc_detach(wl->wlc);
		wl->wlc = NULL;
		wl->pub = NULL;
	}

	osh = wl->osh;
	MFREE(osh, wl, sizeof(wl_info_t));

	osl_detach(osh);
}

/** 
 * attach to the WL device.
 *
 * Attach to the WL device identified by vendor and device parameters.
 * regs is a host accessible memory address pointing to WL device registers.
 *
 * wl_attach is not defined as static because in the case where no bus
 * is defined, wl_attach will never be called, and thus, gcc will issue
 * a warning that this function is defined but not used if we declare
 * it as static.
 */
wl_info_t *
wl_attach(uint16 vendor, uint16 device, ulong regs, uint bustype, void *btparam, uint irq)
{
	struct ifnet *ifp;
	wl_if_t *wlif;
	wl_info_t *wl;
	osl_t *osh;
	int unit;
	uint err;

	unit = wl_found;

	/* Requires pkttag feature */
	osh = osl_attach(btparam, bustype, TRUE);

	/* allocate private info */
	if ((wl = (wl_info_t*) MALLOC(osh, sizeof(wl_info_t))) == NULL) {
		WL_ERROR(("wl%d: malloc wl_info_t, out of memory, malloced %d bytes\n", unit,
			MALLOCED(osh)));
		osl_detach(osh);
		return NULL;
	}
	bzero(wl, sizeof(wl_info_t));

	wl->osh = osh;
	wlif = &(wl->wlif);

	wlif->type = WL_IFTYPE_BSS;
	wlif->wl = wl;
	sprintf(wlif->name, "eth");

	wl->bustype = bustype;
	wl->base_addr = regs;
	wl->regsva = REG_MAP(wl->base_addr, 0);

	if ((wl->wlc = wl_ecos_comm_wlc_attach((void*)wl, vendor, device, unit, 0, osh,
		wl->regsva, wl->bustype, btparam, &err)) == NULL) {
		WL_ERROR(("wl%d: %s driver failed with code %d\n", unit, EPI_VERSION_STR, err));
		goto fail;
	}

	wl->pub = wl_ecos_comm_pub(wl->wlc);

	/* Initialize upper level driver */
	ifp = (struct ifnet *)wlif;
	wl_ecos_comm_get_mac(wl->pub, (char*)&wlif->sc_enaddr);

	wl->vector = irq;
	wl->interrupt_handle = 0;
	cyg_drv_interrupt_create(
			wl->vector,
			0,
			(CYG_ADDRWORD)wl,
			wl_isr,
			wl_dsr,
			&wl->interrupt_handle,
			&wl->interrupt_object);
	if (wl->interrupt_handle != 0) {
		cyg_drv_interrupt_attach(wl->interrupt_handle);
		cyg_drv_interrupt_acknowledge(wl->vector);
	}

	wl->next = wl_list;
	wl_list = wl;

	ifp->if_softc = wlif;
	ifp->if_start = wl_drv_send;
	ifp->if_ioctl = wl_ioctl;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
#ifdef IFF_NOTRAILERS
	ifp->if_flags |= IFF_NOTRAILERS;
#endif
	/* Compatible to linux to use eth1 instead of wl0 */
	sprintf(ifp->if_xname, "eth%d", (wl_found + 1));
	ifp->if_unit = (wl_found + 1);
	ifp->if_name = wlif->name;
	ether_ifattach(ifp, 0);

	wlif->dev = netdev_attach(ifp, wl_dpc);

	wl_found++;
	return wl;

fail:
	wl_free(wl);
	return NULL;
}

/* NETDEVTAB_ENTRY	initial function */
/* ====================================================================== */
static int
wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int rc;
	wl_info_t *wl;

	if (!wl_ecos_comm_wlc_chipmatch(pdev->vendor, pdev->device))
		return (-ENODEV);

#if 0
	diag_printf("%s: find [%04x:%04x] bus %d slot %d func %d irq %d\n", __func__,
	                    pdev->vendor, pdev->device,
	                    pdev->bus->number, PCI_SLOT(pdev->devfn),
	                    PCI_FUNC(pdev->devfn), pdev->irq);
#endif

	rc = pci_enable_device(pdev);
	if (rc) {
		WL_ERROR(("%s: Cannot enable device %d-%d_%d\n", __FUNCTION__,
			pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn)));
		return (-ENODEV);
	}
	pci_set_master(pdev);

	wl = wl_attach(pdev->vendor, pdev->device, pci_resource_start(pdev, 0),
		PCI_BUS, pdev, pdev->irq);

	if (!wl)
		return -ENODEV;

	pci_set_drvdata(pdev, wl);

	return 0;
}

void
wl_txflowcontrol(struct wl_info *wl, struct wl_if *wlif, bool state, int prio)
{
	if (wlif == NULL)
		wlif = &wl->wlif;

	wlif->txqstopped = state;
}

uint wl_reset(wl_info_t *wl)
{
	WL_TRACE(("wl_reset(): reset wl%d\n", wl_ecos_comm_get_unit(wl->pub)));

	wl_ecos_comm_wlc_reset(wl->wlc);

	/* dpc will not be rescheduled */
	wl->resched = 0;

	return 0;
}

void wl_init(wl_info_t *wl)
{
	WL_ERROR(("==== wl_init ===\n"));
	WL_TRACE(("wl_init(): init wl%d\n", wl_ecos_comm_get_unit(wl->pub)));

	wl_reset(wl);
	wl_ecos_comm_wlc_init(wl->wlc);
}

bool
wl_alloc_dma_resources(wl_info_t *wl, uint addrwidth)
{
	return TRUE;
}

void wl_intrson(wl_info_t *wl)
{
	wl_ecos_comm_wlc_intrson(wl->wlc);
}

uint32 wl_intrsoff(wl_info_t *wl)
{
	return wl_ecos_comm_wlc_intrsoff(wl->wlc);
}

void wl_intrsrestore(wl_info_t *wl, uint32 macintmask)
{
	wl_ecos_comm_wlc_intrsrestore(wl->wlc, macintmask);
}

void
wl_event_sync(wl_info_t *wl, char *ifname, void *e)
{
}

int wl_up(wl_info_t *wl)
{
	wl_if_t *wlif;
	struct ifnet *ifp;
	int error = 0, ipl;

	if (wl_ecos_comm_up(wl->pub))
		return (0);

	error = wl_ecos_comm_wlc_up(wl->wlc);

	if (!error) {
		if (wl->interrupt_handle != 0)
			cyg_drv_interrupt_acknowledge(wl->vector);

		cyg_drv_interrupt_unmask(wl->vector);
		/* Bringup interfaces */
		for (wlif = &wl->wlif; wlif; wlif = wlif->next) {
			wl_txflowcontrol(wl, wlif, OFF, ALLPRIO);
			ifp = IFP_WLIF(wlif);
			ipl = splnet();
			ifp->if_flags |= (IFF_RUNNING|IFF_UP);
			ifp->if_flags &= ~IFF_OACTIVE;
			splx(ipl);
		}
		wl->flags |= PRIVIFFG_CANSEND;
	}

	return (error);
}

void wl_down(wl_info_t *wl)
{
	wl_if_t	*wlif;
	struct ifnet *ifp;
	uint callbacks;
	int ipl;

	cyg_drv_interrupt_mask(wl->vector);

	for (wlif = &wl->wlif; wlif; wlif = wlif->next) {
		ifp = IFP_WLIF(wlif);
		ipl = splnet();
		ifp->if_flags &= ~(IFF_UP | IFF_RUNNING);
		splx(ipl);
	}

	wl->flags &= ~PRIVIFFG_CANSEND;

	callbacks = wl->callbacks - wl_ecos_comm_wlc_down(wl->wlc);
	SPINWAIT((wl->callbacks > callbacks), 100 * 1000);
}

static void
wl_start(wl_info_t *wl)
{
	struct ifnet *ifp = IFP_WLIF(&(wl->wlif));
	struct net_device *dev = (wl->wlif.dev);

	WL_TRACE(("wl_start(): start wl%d\n", wl_ecos_comm_get_unit(wl->pub)));

	if (!(wl->flags & PRIVIFFG_ACTIVE)) {
		wl->flags |= PRIVIFFG_ACTIVE;
		wl_up(wl);
	}

	dev->flags |= NETDEV_FLAG_RUNNING;

	wl_drv_send(ifp);
}

static void
wl_stop(wl_info_t *wl)
{
	struct net_device *dev = (wl->wlif.dev);

	WL_TRACE(("wl_stop(): stop wl%d\n", wl_ecos_comm_get_unit(wl->pub)));

	if (wl->flags & PRIVIFFG_ACTIVE) {
		wl_down(wl);
		wl->flags &= ~PRIVIFFG_ACTIVE;
	}

	dev->flags &= ~NETDEV_FLAG_RUNNING;
}


static int
wl_set_multicast_list(wl_if_t *wlif)
{
	wl_info_t *wl;
	struct arpcom *sc;
	struct ifnet *ifp;
	struct ifmultiaddr *ifma;
	int i, maxmultilist, buflen;
	struct maclist *maclist;
	bool allmulti;

	wl = wlif->wl;

	sc = &wlif->sc;
	ifp = &wlif->sc_if;

	allmulti = (ifp->if_flags & IFF_ALLMULTI) ? TRUE: FALSE;
	maxmultilist = wl_ecos_comm_get_maxmultilist();
	buflen = sizeof(struct maclist) + (maxmultilist * ETHER_ADDR_LEN);
	if ((maclist = MALLOC(wl->osh, buflen)) == NULL)
		return -1;
	i = 0;
	for (ifma = ifp->if_multiaddrs.lh_first; ifma;
	     ifma = ifma->ifma_link.le_next) {
		if (i >= maxmultilist) {
			allmulti = TRUE;
			i = 0;
			break;
		}
		bcopy((char *)LLADDR((struct sockaddr_dl *)ifma->ifma_addr),
			&maclist->ea[i++], ETHER_ADDR_LEN);
	}
	maclist->count = i;

	WL_LOCK(wl);
	/* update ALL_MULTICAST common code flag */
	wl_ecos_comm_set_allmulti((void *)wl->wlc, allmulti);
	wl_ecos_comm_wlc_set((void *)wl->wlc, WLC_SET_PROMISC,
		(ifp->if_flags & IFF_PROMISC) ? TRUE: FALSE);
	/* set up address filter for multicasting */
	wl_ecos_comm_set_multicastlist((void *)wl->wlc, maclist, buflen);
	WL_UNLOCK(wl);
	MFREE(wl->osh, maclist, buflen);

	if (ifp->if_flags & IFF_UP) {
		if (wl_ecos_comm_up(wl->pub))
			/* if driver already up, re-init */
			wl_init(wl);
		else
			/* if driver not up, make it up since flag tells to do so */
			wl_up(wl);
	} else if (ifp->if_flags & IFF_RUNNING)
		wl_down(wl);
	return 0;
}

int
wl_ethtool(wl_info_t *wl, void *uaddr, wl_if_t *wlif)
{
	struct ethtool_drvinfo info;
	/* struct ethtool_value edata; */
	uint32 cmd;
	/* uint32 toe_cmpnt, csum_dir; */
	int ret = 0;

	cmd = *(uint32*)uaddr;

	switch (cmd) {
	case ETHTOOL_GDRVINFO:
		bzero(&info, sizeof(info));
		info.cmd = cmd;
		sprintf(info.driver, "wl%d", wl_ecos_comm_get_unit(wl->pub));
		strcpy(info.version, EPI_VERSION_STR);
		bcopy(&info, uaddr, sizeof(info));
		break;
	default:
		return (-EOPNOTSUPP);

	}

	return (ret);
}

static int
wl_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	int s, ret = 0;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifaddr *ifa = (struct ifaddr *)data;
	struct ifaddr *ifa1;
	struct sockaddr_dl *sdl;
	wl_info_t *wl = WL_INFO(ifp);
	wl_if_t *wlif = WLIF_IFP(ifp);

	WL_TRACE(("wl%d: wl_ioctl: cmd 0x%x\n", wl_ecos_comm_get_unit(wl->pub), (int)cmd));

	s = splnet();

	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		switch (ifa->ifa_addr->sa_family) {
#ifdef INET
		case AF_INET:
			wl_start(wl);
			arp_ifinit(&wlif->sc, ifa);
			break;
#endif
		default:
			wl_start(wl);
			break;
		}
		break;

	case SIOCGIFHWADDR:
		ifr->ifr_hwaddr.sa_family = AF_LINK;
		ret = wl_ecos_comm_iovar_op(wl->wlc, "cur_etheraddr",
			(caddr_t)&ifr->ifr_hwaddr.sa_data, ETHER_ADDR_LEN, IOV_GET, wlif->wlcif);
		break;

	case SIOCSIFHWADDR:
		bcopy((char*)&ifr->ifr_hwaddr.sa_data, (char*)&wlif->sc_enaddr, ETHER_ADDR_LEN);
		for (ifa1 = ifp->if_addrlist.tqh_first; ifa1 != 0;
			ifa1 = ifa1->ifa_list.tqe_next) {
			if ((sdl = (struct sockaddr_dl *)ifa1->ifa_addr) &&
				sdl->sdl_family == AF_LINK) {
				sdl->sdl_type = IFT_ETHER;
				sdl->sdl_alen = ifp->if_addrlen;
				bcopy((caddr_t)((struct arpcom *)ifp)->ac_enaddr,
				            LLADDR(sdl), ifp->if_addrlen);
				break;
			}
		}

		/* set driver cur_etheraddr */
		ret = wl_ecos_comm_iovar_op(wl->wlc, "cur_etheraddr",
		                               (caddr_t)((struct arpcom *)ifp)->ac_enaddr,
		                                ETHER_ADDR_LEN, IOV_SET, wlif->wlcif);
		break;

	case SIOCGIFSTATSUD:		/* UD == UPDATE */
		ETH_STATS_INIT( wl );	/* so UPDATE the statistics structure */
		break;

	case SIOCSIFFLAGS:
		if (!(ifp->if_flags & IFF_UP) &&
		    (ifp->if_flags & IFF_RUNNING)) {
			/*
			 * If interface is marked down and it is running, then
			 * stop it.
			 */
			wl_stop(wl);
			ifp->if_flags &= ~IFF_RUNNING;
		}
		else if ((ifp->if_flags & IFF_UP) &&
		             !(ifp->if_flags & IFF_RUNNING)) {
			/*
			 * If interface is marked up and it is stopped, then
			 * start it.
			 */
			wl_start(wl);
			ifp->if_flags |= (IFF_RUNNING|IFF_UP);
		}
		else {
			/*
			 * Reset the interface to pick up changes in any other
			 * flags that affect hardware registers.
			 */
			/* wl_stop(wl); */
			/* wl_start(wl); */
		}

		/* In case of wet mode, we need set driver to receive all multi */
		if (ifr->ifr_flags & IFF_ALLMULTI) {
			ifp->if_flags |= IFF_ALLMULTI;
		}
		else {
			ifp->if_flags &= ~IFF_ALLMULTI;
		}
		wl_ecos_comm_set_allmulti((void *)wl->wlc,
			(ifp->if_flags & IFF_ALLMULTI) ? TRUE: FALSE);
		wl_ecos_comm_wlc_set((void *)wl->wlc, WLC_SET_PROMISC,
			(ifp->if_flags & IFF_PROMISC) ? TRUE: FALSE);
		break;

	case SIOCGIFSTATS:
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		if (wl_ecos_comm_up(wl->pub))
			wl_set_multicast_list(wlif);
		break;

#if defined(SIOCGIFLINK) && defined(STA)
	case SIOCGIFLINK:
	{
		struct phystatus *ps = (struct phystatus *)ifr->ifr_data;

		if (!wl_ecos_comm_get_ap(wl->pub) && wl_ecos_comm_get_associated(wl->pub))
			ps->ps_link = 1;
		else
			ps->ps_link = 0;
		break;
	}
#endif

	case SIOCETHTOOL:
		ret = wl_ethtool(wl, (void*)ifr->ifr_data, wlif);
		break;

	case SIOCDEVPRIVATE:
	{
		wl_ioctl_t *ioc = (wl_ioctl_t *)ifr->ifr_data;

		ret = wl_ecos_comm_wlc_ioctl(wl->wlc, ioc->cmd, ioc->buf, ioc->len, wlif->wlcif);
		break;
	}

	default:
		ret = -1;
		break;
	}

	splx(s);
	return ret;
}

/* wds ioctl handler */
static int
wl_wds_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
#ifdef BCMDBG
	wl_info_t *wl = WL_INFO(ifp);
#endif
	wl_if_t *wds = WLIF_IFP(ifp);
	wl_ioctl_t *ioc;
	int s, error = 0;
	struct ifreq *ifr = (struct ifreq *)data;

	wds = WLIF_IFP(ifp);

	WL_TRACE(("%s: wl_wds_ioctl: cmd 0x%x\n", wl_ifname(wl, wds), (int) cmd));

	s = splnet();

	if (cmd == SIOCDEVPRIVATE) {
		ioc = (wl_ioctl_t *)ifr->ifr_data;
		switch (ioc->cmd) {
		case WLC_WDS_GET_REMOTE_HWADDR:
			if (!ioc->buf || ioc->len < ETHER_ADDR_LEN) {
				error = EINVAL;
				goto done;
			}
			bcopy(&wds->remote.octet, ioc->buf, ETHER_ADDR_LEN);
			break;

		default:
			error = wl_ioctl(ifp, cmd, data);
		}
	}
	else {
		error = wl_ioctl(ifp, cmd, data);
	}

done:
	splx(s);
	return error;
}

static int
wl_cansend(wl_info_t *wl)
{
	wl_if_t *wlif = &wl->wlif;

	if ((wl->flags & PRIVIFFG_CANSEND) && !wlif->txqstopped)
		return 1;
	else
		return 0;
}

/*
 * The last parameter was added for the build. Caller of
 * this function should pass 1 for now.
 */
void
wl_sendup(wl_info_t *wl, wl_if_t *wlif, void *p, int numpkt)
{
	struct mbuf *m;
	struct ifnet *ifp;
	void *osh;
	uint pub_unit;
	struct ether_header *eh;

	osh = wl_ecos_comm_osh(wl->pub);

	if (wlif == NULL)
		wlif = &(wl->wlif);

	if (wlif == NULL) {
		pub_unit = wl_ecos_comm_get_unit(wl->pub);
		/*SAM*/
		PKTFREE(osh, p, false);
		return;
	}

	ifp = IFP_WLIF(wlif);

	if ((ifp->if_flags & IFF_RUNNING) != IFF_RUNNING) {
		/*SAM*/
		PKTFREE(osh, p, false);
		return;
	}

	/* Detach pkttag */
	m = (struct mbuf *)PKTTONATIVE(osh, p);

	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = m->m_len;
	m->m_pkthdr.ext_flag = 0;

	/* Push data into protocol stacks */
	eh = mtod(m, struct ether_header *);
	m_adj(m, sizeof(*eh));

	ether_input(ifp, eh, m);
	ifp->if_ipackets++;

	return;
}

static cyg_uint32 wl_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	wl_info_t *wl = (wl_info_t *)data;
	bool ours, wantdsr;

	if (!wl_ecos_comm_up(wl->pub))
		return 0;

	if (!(ours = wl_ecos_comm_wlc_isr(wl->wlc, &wantdsr))) {
		cyg_drv_interrupt_acknowledge(wl->vector);
		return (0);
	}

	if (wantdsr) {
		wl_ecos_comm_wlc_intrsoff_isr(wl->wlc);
		cyg_drv_interrupt_acknowledge(wl->vector);
		return (CYG_ISR_CALL_DSR);
	}

	return (CYG_ISR_HANDLED);
}

static void wl_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	wl_info_t *wl = (wl_info_t *)data;
	struct net_device *dev = (wl->wlif.dev);

	if (!wl_ecos_comm_up(wl->pub))
		return;

	if (dev) {
		dev->flags |= NETDEV_FLAG_INTPENDING;
		kern_schedule_dsr();
	}
}

char * wl_ifname(wl_info_t *wl, struct wl_if *wlif)
{
	if (wlif)
		return (IFP_WLIF(wlif))->if_xname;
	else
		return (IFP_WLIF(&(wl->wlif)))->if_xname;
}

void wl_dump_ver(wl_info_t *wl, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "wl%d: %s %s version %s\n", wl_ecos_comm_get_unit(wl->pub),
		__DATE__, __TIME__, EPI_VERSION_STR);
}

void wl_link_up(wl_info_t *wl)
{
	WL_TRACE(("wl_link_up(): link up wl%d\n", wl_ecos_comm_get_unit(wl->pub)));
}

void wl_link_down(wl_info_t *wl)
{
	WL_TRACE(("wl_link_down(): link down wl%d\n", wl_ecos_comm_get_unit(wl->pub)));
}

void wl_event(struct wl_info *wl, char *ifname, void *e)
{
	uint32 event_type;
	uint8  event_flags;
	struct ether_addr *event_addr;

	event_type = wl_ecos_comm_get_event_type(e);
	event_flags = wl_ecos_comm_get_event_flags(e);

	switch (event_type) {
	case WLC_E_LINK:
	case WLC_E_NDIS_LINK:
		if (event_flags & WLC_EVENT_MSG_LINK)
			wl_link_up(wl);
		else
			wl_link_down(wl);
		break;
	case WLC_E_MIC_ERROR:
		event_addr = wl_ecos_comm_get_event_addr(e);
		wl_mic_error(wl, event_addr, event_flags & WLC_EVENT_MSG_GROUP,
		                       event_flags & WLC_EVENT_MSG_FLUSHTXQ);
		break;
	default:
		break;
	}
}

static void wl_mic_error(wl_info_t *wl, struct ether_addr *ea, bool group, bool flush_txq)
{
#ifdef	WPAPSK
	if (wl_ecos_comm_wlc_sup_mic_error(wl->wlc, group))
		return;
#endif
}

/* ====================================================================== */
/*  Timer function for wlc */
static void
wl_timer_func(int p1)
{

	wl_timer_t *t;

	t = (wl_timer_t*)p1;

	if (t->set) {
		if (t->periodic) {
			t->wl->callbacks++;
			t->set = TRUE;
			t->timer = timeout((timeout_fun *)wl_timer_func, t, t->tick);
		}
		else {
			t->set = FALSE;
		}
		t->fn(t->arg);
	}

	t->wl->callbacks--;
}

wl_timer_t*
wl_init_timer(wl_info_t *wl, void (*fn)(void *wlc), void *arg, const char *name)
{
	wl_timer_t *t;

	if ((t = (wl_timer_t*)malloc(sizeof(wl_timer_t))) == NULL)
		return 0;

	t->wl = wl;
	t->fn = fn;
	t->arg = arg;
	t->set = FALSE;
	t->periodic = FALSE;
	t->tick = 0;
	t->timer = 0;

#ifdef BCMDBG
	if (name) {
		if ((t->name = malloc(strlen(name) + 1)))
			strcpy(t->name, name);
	}
	else
		t->name = NULL;
#endif

	return (t);
}

void
wl_add_timer(wl_info_t *wl, wl_timer_t *t, uint ms, int periodic)
{
	cyg_tick_count_t ostick;

	if (!t)
		return;

	if (t->set == TRUE)
		return;

	ostick = ms / 10;
	t->tick = ostick;
	t->periodic = (bool)periodic;
	t->set = TRUE;
	wl->callbacks++;

	t->timer = timeout((timeout_fun *)wl_timer_func, t, t->tick);
}

bool
wl_del_timer(wl_info_t *wl, wl_timer_t *t)
{
	if (!t)
		return TRUE;

	if (t->set) {
		t->set = FALSE;
		untimeout((timeout_fun *)wl_timer_func, t);
		wl->callbacks--;
	}
	return TRUE;
}

void
wl_free_timer(wl_info_t *wl, wl_timer_t *t)
{
	if (!t)
		return;

	if (t->timer)
		untimeout((timeout_fun *)wl_timer_func, t);
}


/* ====================================================================== */
/*  Interface function */
static struct wl_if *
wl_alloc_if(wl_info_t *wl, uint unit, struct wlc_if* wlcif, struct ether_addr *remote)
{
	wl_if_t *p;
	wl_if_t *wlif;
	struct ifnet *ifp;
	uint pub_unit;

	/* allocate the wl_if_t structure */
	if ((wlif = (wl_if_t*) malloc(sizeof(wl_if_t))) == NULL) {
		return NULL;
	}

	pub_unit = wl_ecos_comm_get_unit(wl->pub);

	bzero((char*)wlif, sizeof(wl_if_t));

	ifp = (struct ifnet *)wlif;
	sprintf(ifp->if_xname, "%s%d.%d", (remote) ? WDS_DEV_NAME : WL_DEV_NAME, pub_unit, unit);

	wlif->wl = wl;
	wlif->wlcif = wlcif;
	sprintf(wlif->name, "%s%d.", (remote) ? WDS_DEV_NAME : WL_DEV_NAME, pub_unit);
	wlif->unit = unit;
	wlif->type = (remote) ? WL_IFTYPE_WDS : WL_IFTYPE_BSS;
	if (remote) bcopy((char*)remote, (char*)&wlif->remote, ETHER_ADDR_LEN);


	/* add the interface to the linked list */
	for (p = (wl_if_t *)&wl->wlif; p->next != NULL; p = p->next)
		;
	p->next = wlif;

	return wlif;
}

/* free a wl_if structure; unlink from the list of interfaces and deallocate */
static void
wl_free_if(wl_info_t *wl, wl_if_t *wlif)
{
	int ipl;
	wl_if_t *p;
	struct ifnet *ifp = (struct ifnet*)wlif;

	if (wlif == NULL)
		return;

	if (ifp->if_flags | (IFF_UP|IFF_RUNNING))
		ifp->if_flags &= ~(IFF_UP|IFF_RUNNING);

	/* remove the interface from the interface linked list */
	for (p = (wl_if_t *)&wl->wlif; p != NULL; p = p->next) {
		if (p->next == wlif) {
			p->next = p->next->next;
			break;
		}
	}

	ipl = splnet();

	ether_ifdetach(ifp, 0);
	if_detach(ifp);

	splx(ipl);

	free(wlif);
}

static void
_wl_add_if(wl_if_t *wlif)
{
	wl_info_t *wl = wlif->wl;
	struct ifnet *ifp = (struct ifnet*)wlif;
	int status, ipl;

	ipl = splnet();

	wl_ecos_comm_get_mac(wl->pub, (char*)&wlif->sc_enaddr);
	ifp->if_softc = wlif;
	ifp->if_start = wl_drv_send;
	ifp->if_name = wlif->name;
	ifp->if_unit = wlif->unit;
	ifp->if_ioctl = (wlif->type == WL_IFTYPE_WDS) ? wl_wds_ioctl : wl_ioctl;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
#ifdef IFF_NOTRAILERS
	ifp->if_flags |= IFF_NOTRAILERS;
#endif
	ether_ifattach(ifp, 0);

	splx(ipl);

	/* Special handling for truly dynamic (WDS) interfaces */
	if (wlif->type == WL_IFTYPE_WDS) {
		char buf[32];
		ifp->if_flags |= IFF_UP;
		status = br_add_if(0, ifp->if_xname);

		/* Inform driver to send up new WDS link event */
		strcpy(buf, "wds_enable");
		if (wl_ecos_comm_wlc_ioctl(wl->wlc, WLC_SET_VAR, buf, sizeof(buf), wlif->wlcif)) {
			WL_ERROR(("wl%d: _wl_add_if: failed WDS_ENABLE for %s\n",
				wl_ecos_comm_get_unit(wl->pub),
				wl_ifname(wl, wlif)));
		}
	}

	return;
}

static void
_wl_del_if(wl_if_t *wlif)
{
	wl_free_if(wlif->wl, wlif);
}

wl_if_t*
wl_add_if(wl_info_t *wl, struct wlc_if* wlcif, uint unit, struct ether_addr *remote)
{
	wl_if_t *wlif;
	uint pub_unit;

	/* allocate the virtual interface structure */
	pub_unit = wl_ecos_comm_get_unit(wl->pub);
	wlif = wl_alloc_if(wl, unit, wlcif, remote);
	if (wlif == NULL) {
		return NULL;
	}

	if (wlif->type == WL_IFTYPE_WDS) {
		/* wait for primary interface up */
		timeout((timeout_fun *)_wl_add_if, wlif, 1);
	} else
		_wl_add_if(wlif);


	return wlif;
}

void
wl_del_if(wl_info_t *wl, wl_if_t *wlif)
{
	struct ifnet *ifp = (struct ifnet*)wlif;

	if (wlif->type == WL_IFTYPE_WDS)
		br_del_if(0, ifp->if_xname);

	wlif->wlcif = NULL;
	timeout((timeout_fun *)_wl_del_if, wlif, 1);
	return;
}

/* Turn monitor mode on or off . This function assumes
 * it is called within the perimeterlock
*/
void
wl_set_monitor(wl_info_t *wl, int val)
{
	return;
}

/* Sends encasulated packets up the monitor interface */
void
wl_monitor(wl_info_t *wl, struct wl_rxsts *rxsts, void *p)
{
	return;
}
