/*
 *  Copyright 2007, Broadcom Corporation
 *   All Rights Reserved.
 *
 *  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 *  the contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of Broadcom Corporation.
 */
/*
 * eCos device driver wrapper for
 * Broadcom BCM47XX 10/100 Mbps Ethernet Controller
 *
 * Copyright (C) 2007 Broadcom Corporation
 *
 * $Id: et_ecos_common.c,v 1.4.64.1 2010-12-21 02:38:53 Exp $
 */

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmdefs.h>
#include <osl.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <bcmdevs.h>
#include <bcmenetmib.h>
#include <bcmenetrxh.h>
#include <bcmenetphy.h>
#include <etioctl.h>
#include <bcmutils.h>
#include <et_dbg.h>
#include <hndsoc.h>
#include <bcmgmacrxh.h>
#include <etc.h>


bool
et_chipmatch(uint vendor, uint device)
{
	if  (!etc_chipmatch(vendor,  device))
		return (FALSE);
	else
		return (TRUE);
}

void *
et_attach(void *et_info, uint vendor, uint device, uint unit, void *osh, void *base)
{
	return etc_attach(et_info, vendor, device, unit, osh, base);
}

void
et_detach(void *et_info)
{
	etc_detach(et_info);
}

void
et_get_hwaddr(void *etc, char *addr)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	bcopy(&etc_info->cur_etheraddr, addr, ETHER_ADDR_LEN);
}

void
et_set_hwaddr(void *etc, char *addr)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	bcopy(addr, &etc_info->cur_etheraddr, ETHER_ADDR_LEN);
}

void
et_promisc(void *etc, bool promisc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_info->promisc = promisc;
}

void
et_allmulti(void *etc, bool allmulti)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_info->allmulti = allmulti;
	if (allmulti)
		etc_info->nmulticast = 0;
}

int
et_get_linkstate(void *etc, int port_num)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	uint16 portmask = (1 << port_num);
	int argv[2];
	uint16 status;
	
	if (etc_info->robo) {
		argv[0] = (0x01 << 16) | 0x00;
		argv[1] = 0;
		
		if (etc_ioctl(etc_info, ETCROBORD, argv) == 0) {
			status = (argv[1] & portmask);
			return (status == portmask);
		}
	} else {
		argv[0] = 1;
		argv[1] = 0;
		if (etc_ioctl(etc_info, ETCPHYRD, argv) == 0)
			return ((argv[1] & STAT_LINK) == STAT_LINK);
	}
	
	return 0;
}

int
et_get_linkspeed(void *etc, int port_num)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	uint16 portmask = (1 << port_num);
	int argv[2];
	uint16 speed;
	
	argv[0] = (0x01 << 16) | 0x04;
	argv[1] = -1;
	etc_ioctl(etc_info, ETCROBORD, argv);
	if (argv[1] != -1) {
		speed = (argv[1] & portmask);
		return ((speed == portmask)? 100:10);
	}
	
	return 100;
}

int
et_get_duplex(void *etc, int port_num)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	uint16 portmask = (1 << port_num);
	int argv[2];
	uint16 duplex;
	
	argv[0] = (0x01 << 16) | 0x06;
	argv[1] = -1;
	etc_ioctl(etc_info, ETCROBORD, argv);
	if (argv[1] != -1) {
		duplex = (argv[1] & portmask);
		return ((duplex == portmask)? 1:0);
	}
	
	return 1;
}

#define PHY_FORCED_SPEED	(1<<13)
#define PHY_AN_ENABLE		(1<<12)
#define PHY_AN_RESTART		(1<<9)
#define PHY_DUPLEX_MODE		(1<<8)

int
et_set_speed(void *etc, int port_num, int speed)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	int argv[2];
	uint16 port_control;
	
	if (port_num < 0 || port_num > 4)
		return -1;
	/* read the original value */
	argv[0] = (port_num << 16) | 0x00;
	argv[1] = -1;
	etc_ioctl(etc_info, ETCPHYRD2, argv);
	port_control = argv[1];
	
	diag_printf("adv (before write) = %04x\n", port_control);
	
	if (speed == ET_10HALF) {
		diag_printf("set port %d to %s\n", port_num, "10HALF");
		
		port_control &= ~(PHY_FORCED_SPEED|PHY_DUPLEX_MODE|PHY_AN_ENABLE);
	} else if (speed == ET_10FULL) {
		diag_printf("set port %d to %s\n", port_num, "10FULL");
		port_control &= ~(PHY_FORCED_SPEED|PHY_AN_ENABLE);
		port_control |= PHY_DUPLEX_MODE;
	} else if (speed == ET_100HALF) {
		diag_printf("set port %d to %s\n", port_num, "100HALF");
		port_control &= ~(PHY_DUPLEX_MODE|PHY_AN_ENABLE);
		port_control |= PHY_FORCED_SPEED;
	} else if (speed == ET_100FULL) {
		diag_printf("set port %d to %s\n", port_num, "100FULL");
		port_control &= ~(PHY_AN_ENABLE);
		port_control |= (PHY_FORCED_SPEED|PHY_DUPLEX_MODE);
	} else {
		diag_printf("set port %d to %s\n", port_num, "AUTO");
		port_control &= ~(PHY_FORCED_SPEED|PHY_DUPLEX_MODE|PHY_AN_ENABLE);
		port_control |= (PHY_AN_ENABLE|PHY_FORCED_SPEED|PHY_AN_RESTART);
	}
	
	argv[1] = port_control;
	diag_printf("write port_control = %04x\n", port_control);
	etc_ioctl(etc_info, ETCPHYWR2, argv);
	
	
	argv[1] = -1;
	etc_ioctl(etc_info, ETCPHYRD2, argv);
	port_control = argv[1];
	
	diag_printf("port_control (after write) = %04x\n", port_control);
	
	return 0;
}

int
et_txdesc_num(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return (*etc_info->txavail[TX_Q0]);
}

void
et_xmit(void *etc, void *p, int len)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	(*etc_info->chops->tx)(etc_info->ch, (void *)p);
	
	etc_info->txframe++;
	etc_info->txbyte += len;
}

void
et_recv_update(void *etc, int len)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_info->rxframe++;
	etc_info->rxbyte += len;
}

void
et_show_errors(void *etc, struct ether_addr *src, uint16 flags)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	uchar eabuf[32];
	
	bcm_ether_ntoa(src, (char *)eabuf);
	if (flags & RXF_NO) {
		diag_printf("et%d: rx: crc error (odd nibbles) from %s\n", etc_info->unit, eabuf);
	}
	if (flags & RXF_RXER) {
		diag_printf("et%d: rx: symbol error from %s\n", etc_info->unit, eabuf);
	}
	if ((flags & RXF_CRC) == RXF_CRC) {
		diag_printf("et%d: rx: crc error from %s\n", etc_info->unit, eabuf);
	}
	if (flags & RXF_OV) {
		diag_printf("et%d: rx: fifo overflow\n", etc_info->unit);
	}
}

void *
et_recv_pkt(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	return ((*chops->rx)(etc_info->ch));
}

void
et_rxfill(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	(*chops->rxfill)(etc_info->ch);
}

void
et_txreclaim(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	(*chops->txreclaim)(etc_info->ch, FALSE);
}

bool
et_chip_errors(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	return (*chops->errors)(etc_info->ch);
}

void
et_enable_int(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	(*chops->intrson)(etc_info->ch);
}

void
et_disable_int(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	
	(*chops->intrsoff)(etc_info->ch);
}

bool
et_chip_up(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return etc_info->up;
}

uint
et_int_events(void *etc, bool in_isr)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	struct chops *chops = etc_info->chops;
	void *ch = etc_info->ch;
	
	return (*chops->getintrevents)(ch, in_isr);
}

void
et_chip_init(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_init(etc_info, ET_INIT_DEF_OPTIONS);

	/* We do not want to invoke watchdog function to get link status;
	 * statically configure the full-duplex mode right after chip_init.
         */
	etc_info->linkstate = TRUE;
	etc_info->duplex = 1;
	
	(*etc_info->chops->duplexupd)(etc_info->ch);
}

void
et_chip_reset(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_reset(etc_info);
}

void
et_chip_open(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_up(etc_info);
}

void
et_chip_close(void *etc, int reset)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_down(etc_info, reset);
}

bool
et_nicmode(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return (etc_info->nicmode);
}

uint
et_etc_unit(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;

	return (etc_info->unit);
}

uint
et_core_unit(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return (etc_info->coreunit);
}

uint16
et_chip_phyrd(void *etc, uint phyaddr, uint reg)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return etc_info->chops->phyrd(etc_info->ch, phyaddr, reg);
}

void
et_chip_phywr(void *etc, uint phyaddr, uint reg, uint16 val)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_info->chops->phywr(etc_info->ch, phyaddr, reg, val);
}

void
et_chip_watchdog(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_watchdog(etc_info);
}

int
et_rx_hwoffset(void)
{
	return HWRXOFF;
}

uint
et_rx_event(uint events)
{
	return (events & INTR_RX);
}

uint
et_tx_event(uint events)
{
	return (events & INTR_TX);
}

uint
et_error_event(uint events)
{
	return (events & INTR_ERROR);
}

uint
et_new_event(uint events)
{
	return (events & INTR_NEW);
}

bool
et_qos_pkt(void *etc)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return (etc_info->qos);
}

void
et_qos_config(void *etc, uint val)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	etc_qos(etc_info, val);
	return;
}

int
et_robo_read(void *etc, void *argv)
{
	etc_info_t *etc_info = (etc_info_t *)etc;

	return (etc_ioctl(etc_info, ETCROBORD, argv));
}

int
et_robo_write(void *etc, void *argv)
{
	etc_info_t *etc_info = (etc_info_t *)etc;
	
	return (etc_ioctl(etc_info, ETCROBOWR, argv));
}

int
et_etc_ioctl(void *etc, int cmd, void *argv)
{
	etc_info_t *etc_info = (etc_info_t *)etc;

	return (etc_ioctl(etc_info, cmd, argv));
}

uint16
et_etc_rxh_flags(void *etc, bcmenetrxh_t *rxh)
{
	etc_info_t *etc_info = (etc_info_t *)etc;

	return RXH_FLAGS(etc_info, rxh);
}

#ifdef BCMDBG
void
et_dbg_dump(void *etc, struct bcmstrbuf *b)
{
	etc_info_t *etc_info = (etc_info_t *)etc;

	etc_dump(etc_info, b);
}
#endif
