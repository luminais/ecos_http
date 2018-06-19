/*
 * PCI bios functions for BCM board
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: pcibios.c,v 1.3 2010-05-31 09:22:22 Exp $
 */

#include <typedefs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <siutils.h>
#include <hndpci.h>
#include <pcicfg.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <pci_core.h>
#include <pcie_core.h>

#include <cyg/kernel/kapi.h>
#include <cyg/hal/pci/bcmpci.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/diag.h>


/* Global SI handle */
extern si_t *bcm947xx_sih;

/* Convenience */
#define sih bcm947xx_sih

static u32 pci_iobase = 0x100;
static u32 pci_membase = SI_PCI_MEM;

static int
sbpci_read_config_byte(struct pci_dev *dev, int where, u8 *value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	*value = 0xff;
	ret = hndpci_read_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, value, sizeof(*value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_read_config_word(struct pci_dev *dev, int where, u16 *value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	*value = 0xffff;
	ret = hndpci_read_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, value, sizeof(*value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_read_config_dword(struct pci_dev *dev, int where, u32 *value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	*value = 0xffffffff;
	ret = hndpci_read_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, value, sizeof(*value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_write_config_byte(struct pci_dev *dev, int where, u8 value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	ret = hndpci_write_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, &value, sizeof(value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_write_config_word(struct pci_dev *dev, int where, u16 value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	ret = hndpci_write_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, &value, sizeof(value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_write_config_dword(struct pci_dev *dev, int where, u32 value)
{
	unsigned long __state;
	int ret;

	HAL_DISABLE_INTERRUPTS(__state);
	ret = hndpci_write_config(sih, dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), where, &value, sizeof(value));
	HAL_RESTORE_INTERRUPTS(__state);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static struct pci_ops pcibios_ops = {
	sbpci_read_config_byte,
	sbpci_read_config_word,
	sbpci_read_config_dword,
	sbpci_write_config_byte,
	sbpci_write_config_word,
	sbpci_write_config_dword
};

void
pcibios_init(void)
{
	sbpciregs_t *pci;
	uint32 chip, chippkg;
	sbpcieregs_t *pcie;
	uint32 size, offset, value;

	if (!(sih = si_kattach(SI_OSH))) {
		diag_printf("%s: si_kattach failed\n", __func__);
		return;
	}
	hndpci_init(sih);

	/*
	 * The 4712 in 200pin package does not bond out PCI.
	 */
	chip = sih->chip;
	chippkg = sih->chippkg;
	if (!((chip == BCM4712_CHIP_ID) && (chippkg == BCM4712SMALL_PKG_ID))) {
		/* PCI core configuration */
		if ((pci = (sbpciregs_t *) si_setcore(sih, PCI_CORE_ID, 0)) != NULL) {
			/* Cannot access high PCI memory window */
			W_REG(NULL, &pci->sbtopci0, SBTOPCI_MEM | pci_membase);
		} else if ((pcie = (sbpcieregs_t *)si_setcore(sih, PCIE_CORE_ID, 0))) {
			/* Enable a second function on the PCI-E bridge */
			W_REG(NULL, &pcie->sprom[0], 0x3);

			/* Configure function 1 BAR1 (128 MB window into swap space) */
			W_REG(NULL, &pcie->sprom[60], 0xf);
			/* Detect BAR sizes and decide the BAR1 offset */
			hndpci_read_config(sih, 1, 0, 1, PCI_BASE_ADDRESS_0, &size, sizeof(size));
			if ((size & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64)
				offset = PCI_BASE_ADDRESS_0 + 8;
			else
				offset = PCI_BASE_ADDRESS_0 + 4;
			value = SI_SDRAM_SWAPPED;
			hndpci_write_config(sih, 1, 0, 1, offset, &value, sizeof(value));
			hndpci_write_config(sih, 1, 0, 1, PCI_BAR1_WIN, &value, sizeof(value));

			/* Enable PCI bridge bus mastering and memory space */
			value = PCI_COMMAND_MEMORY|PCI_COMMAND_MASTER;
			hndpci_write_config(sih, 1, 0, 1, PCI_COMMAND, &value, sizeof(value));
		}
	}

	/* Scan the SB bus */
	pci_scan_bus(0, &pcibios_ops, NULL);
}

void
pcibios_fixup_bus(struct pci_bus *b)
{
	struct list_head *ln;
	struct pci_dev *d;
	struct resource *res;
	int pos, size;
	u32 *base;
	u8 irq;
	u32 type;

	diag_printf("PCI: Fixing up bus %d\n", b->number);

	/* Fix up SB */
	if (b->number == 0) {
		for (ln = b->devices.next; ln != &b->devices; ln = ln->next) {
			d = pci_dev_b(ln);
			/* Fix up interrupt lines */
			pci_read_config_byte(d, PCI_INTERRUPT_LINE, &irq);
			d->irq = irq;
			pci_write_config_byte(d, PCI_INTERRUPT_LINE, d->irq);
		}
	} else {
		/* Fix up external PCI */
		for (ln = b->devices.next; ln != &b->devices; ln = ln->next) {
			d = pci_dev_b(ln);
			/* Fix up resource bases */
			for (pos = 0; pos < 6; pos++) {
				res = &d->resource[pos];
				base = (res->flags & IORESOURCE_IO) ? &pci_iobase : &pci_membase;
				if (res->end) {
					size = res->end - res->start + 1;
					if (*base & (size - 1))
						*base = (*base + size) & ~(size - 1);
					res->start = *base;
					res->end = res->start + size - 1;
					*base += size;
					pci_write_config_dword(d,
						PCI_BASE_ADDRESS_0 + (pos << 2), res->start);
					/* Detect BAR sizes and decide the BAR1 offset */
					pci_read_config_dword(d, PCI_BASE_ADDRESS_0, &type);
					if ((type & PCI_BASE_ADDRESS_MEM_TYPE_MASK) == PCI_BASE_ADDRESS_MEM_TYPE_64) {
						pos++;
						pci_write_config_dword(d,
							PCI_BASE_ADDRESS_0 + (pos << 2), 0);
					}
				}
				/* Fix up PCI bridge BAR0 only */
				if (b->number == 1 && PCI_SLOT(d->devfn) == 0)
					break;
			}
			/* Fix up interrupt lines */
			if (pci_find_device(VENDOR_BROADCOM, PCI_CORE_ID, NULL))
				d->irq = (pci_find_device(VENDOR_BROADCOM, PCI_CORE_ID, NULL))->irq;
			else if (pci_find_device(VENDOR_BROADCOM, PCIE_CORE_ID, NULL))
				d->irq = (pci_find_device(VENDOR_BROADCOM, PCIE_CORE_ID,
				NULL))->irq;
			pci_write_config_byte(d, PCI_INTERRUPT_LINE, d->irq);
		}
		hndpci_arb_park(sih, PCI_PARK_NVRAM);
	}
}

unsigned int
pcibios_assign_all_busses(void)
{
	return 1;
}

void
pcibios_align_resource(void *data, struct resource *res,
	unsigned long size, unsigned long align)
{
}

int
pcibios_enable_resources(struct pci_dev *dev)
{
	u16 cmd, old_cmd;
	int idx;
	struct resource *r;

	/* External PCI only */
	if (dev->bus->number == 0)
		return 0;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	old_cmd = cmd;
	for (idx = 0; idx < 6; idx++) {
		r = &dev->resource[idx];
		if (r->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (r->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}
	if (dev->resource[PCI_ROM_RESOURCE].start)
		cmd |= PCI_COMMAND_MEMORY;
	if (cmd != old_cmd) {
		diag_printf("PCI: Enabling device %s (%04x -> %04x)\n",
			dev->slot_name, old_cmd, cmd);
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

int
pcibios_enable_device(struct pci_dev *dev, int mask)
{
	uint coreidx;
	void *regs;

	/* External PCI device enable */
	if (dev->bus->number != 0)
		return pcibios_enable_resources(dev);

	/* These cores come out of reset enabled */
	if (dev->device == MIPS_CORE_ID ||
	    dev->device == MIPS33_CORE_ID ||
	    dev->device == MIPS74K_CORE_ID ||
	    dev->device == EXTIF_CORE_ID ||
	    dev->device == CC_CORE_ID)
		return 0;

	coreidx = si_coreidx(sih);
	regs = si_setcoreidx(sih, PCI_SLOT(dev->devfn));
	if (!regs)
		return PCIBIOS_DEVICE_NOT_FOUND;

	/* 
	 * The USB core requires a special bit to be set during core
	 * reset to enable host (OHCI) mode. Resetting the SB core in
	 * pcibios_enable_device() is a hack for compatibility with
	 * vanilla usb-ohci so that it does not have to know about
	 * SB. A driver that wants to use the USB core in device mode
	 * should know about SB and should reset the bit back to 0
	 * after calling pcibios_enable_device().
	 */
	if (si_coreid(sih) == USB_CORE_ID) {
		si_core_disable(sih, si_core_cflags(sih, 0, 0));
		si_core_reset(sih, 1 << 13, 0);
	}
	else
		si_core_reset(sih, 0, 0);

	si_setcoreidx(sih, coreidx);

	return 0;
}

void
pcibios_update_resource(struct pci_dev *dev, struct resource *root,
	struct resource *res, int resource)
{
	unsigned long where, size;
	u32 reg;

	/* External PCI only */
	if (dev->bus->number == 0)
		return;

	where = PCI_BASE_ADDRESS_0 + (resource * 4);
	size = res->end - res->start;
	pci_read_config_dword(dev, where, &reg);
	reg = (reg & size) | (((u32)(res->start - root->start)) & ~size);
	pci_write_config_dword(dev, where, reg);
}

static void
quirk_sbpci_bridge(struct pci_dev *dev)
{
	if (dev->bus->number != 1 || PCI_SLOT(dev->devfn) != 0)
		return;

	diag_printf("PCI: Fixing up bridge\n");

	/* Enable PCI bridge bus mastering and memory space */
	pci_set_master(dev);
	pcibios_enable_resources(dev);

	/* Enable PCI bridge BAR1 prefetch and burst */
	pci_write_config_dword(dev, PCI_BAR1_CONTROL, 3);
}

struct pci_fixup pcibios_fixups[] = {
	{ PCI_FIXUP_HEADER, PCI_ANY_ID, PCI_ANY_ID, quirk_sbpci_bridge },
	{ 0 }
};
