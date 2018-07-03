/*-
 * Copyright (c) 2012 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cyg/io/usb_sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <cyg/io/usb_sys/stdint.h>
#include <cyg/io/usb_sys/stddef.h>
#include <cyg/io/usb_sys/param.h>
#include <cyg/io/usb_sys/queue.h>
#include <cyg/io/usb_sys/types.h>
#include <cyg/io/usb_sys/systm.h>
#include <cyg/io/usb_sys/kernel.h>
#include <cyg/io/usb_sys/bus.h>
#include <cyg/io/usb_sys/module.h>
#include <cyg/io/usb_sys/lock.h>
#include <cyg/io/usb_sys/mutex.h>
#include <cyg/io/usb_sys/condvar.h>
#include <cyg/io/usb_sys/sysctl.h>
#include <cyg/io/usb_sys/sx.h>
#include <cyg/io/usb_sys/unistd.h>
#include <cyg/io/usb_sys/callout.h>
#include <cyg/io/usb_sys/malloc.h>
#include <cyg/io/usb_sys/priv.h>
#include <cyg/io/usb_sys/rman.h>

#include <cyg/io/usb_dev/usb/usb.h>
#include <cyg/io/usb_dev/usb/usbdi.h>

#include <cyg/io/usb_dev/usb/usb_core.h>
#include <cyg/io/usb_dev/usb/usb_busdma.h>
#include <cyg/io/usb_dev/usb/usb_process.h>
#include <cyg/io/usb_dev/usb/usb_util.h>

#include <cyg/io/usb_dev/usb/usb_controller.h>
#include <cyg/io/usb_dev/usb/usb_bus.h>

#include <cyg/io/usb_dev/usb/controller/dwc_otg.h>

cyg_interrupt 	int2;
cyg_handle_t	int2_handle;
cyg_vector_t	int2_vector = 33;
cyg_priority_t	int2_priority = 0;

static device_probe_t dwc_otg_probe;
static device_attach_t dwc_otg_attach;
static device_detach_t dwc_otg_detach;

struct dwc_otg_super_softc {
	struct dwc_otg_softc sc_otg;	/* must be first */
};

static int
dwc_otg_probe(device_t dev)
{
	device_set_desc(dev, "DWC OTG 2.0 integrated USB controller");
	return (0);
}

static int
dwc_otg_attach(device_t dev)
{
	struct dwc_otg_super_softc *sc = device_get_softc(dev);
	int err;
	int rid;
	
	/*winfred_wang*/
	REG32(0xb8000010)=(REG32(0xb8000010)|(1<<29));
	REG32(0xb8000034)=(REG32(0xb8000034)|(1<<12)|(1<<22))&(~(1<<18));
	REG32(0xb8000090)=(REG32(0xb8000090)|(1<<8)|(1<<10)|(1<<19)|(1<<21))&((~(1<<9))&(~(1<<20)));

	/* initialise some bus fields */
	sc->sc_otg.sc_bus.parent = dev;
	sc->sc_otg.sc_bus.devices = sc->sc_otg.sc_devices;
	sc->sc_otg.sc_bus.devices_max = DWC_OTG_MAX_DEVICES;

	/* get all DMA memory */
	if (usb_bus_mem_alloc_all(&sc->sc_otg.sc_bus,
	    USB_GET_DMA_TAG(dev), NULL)) {
		return (ENOMEM);
	}
#if 0
	rid = 0;
	sc->sc_otg.sc_io_res =
	    bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);

	if (!(sc->sc_otg.sc_io_res)) {
		err = ENOMEM;
		goto error;
	}
	sc->sc_otg.sc_io_tag = rman_get_bustag(sc->sc_otg.sc_io_res);
	sc->sc_otg.sc_io_hdl = rman_get_bushandle(sc->sc_otg.sc_io_res);
	sc->sc_otg.sc_io_size = rman_get_size(sc->sc_otg.sc_io_res);

	rid = 0;
	sc->sc_otg.sc_irq_res =
	    bus_alloc_resource_any(dev, SYS_RES_IRQ, &rid, RF_ACTIVE);
	if (sc->sc_otg.sc_irq_res == NULL)
		goto error;
#endif



	sc->sc_otg.sc_bus.bdev = device_add_child(dev, "usbus", -1);
	if (sc->sc_otg.sc_bus.bdev == NULL)
		goto error;

	device_set_ivars(sc->sc_otg.sc_bus.bdev, &sc->sc_otg.sc_bus);
	
#if 0
	err = bus_setup_intr(dev, sc->sc_otg.sc_irq_res, INTR_TYPE_BIO | INTR_MPSAFE,
	    NULL, (driver_intr_t *)dwc_otg_interrupt, sc, &sc->sc_otg.sc_intr_hdl);
	if (err) {
		sc->sc_otg.sc_intr_hdl = NULL;
		goto error;
	}
#endif
	
	cyg_interrupt_create(int2_vector,int2_priority,sc,&dwc_otg_interrupt_isr,&dwc_otg_interrupt_dsr,&int2_handle,&int2);
	cyg_interrupt_attach(int2_handle);
	cyg_interrupt_unmask(int2_vector);
	
	err = dwc_otg_init(&sc->sc_otg);
	if (err == 0) {
		err = device_probe_and_attach(sc->sc_otg.sc_bus.bdev);
	}
	if (err)
		goto error;
	return (0);

error:
	dwc_otg_detach(dev);
	return (ENXIO);
}

static int
dwc_otg_detach(device_t dev)
{
	struct dwc_otg_super_softc *sc = device_get_softc(dev);
	device_t bdev;
	int err;

	if (sc->sc_otg.sc_bus.bdev) {
		bdev = sc->sc_otg.sc_bus.bdev;
		device_detach(bdev);
		device_delete_child(dev, bdev);
	}
	/* during module unload there are lots of children leftover */
	device_delete_children(dev);

	{
		/*
		 * only call dwc_otg_uninit() after dwc_otg_init()
		 */
		dwc_otg_uninit(&sc->sc_otg);

		cyg_interrupt_delete(int2_handle);
		sc->sc_otg.sc_intr_hdl = NULL;
	}
	usb_bus_mem_free_all(&sc->sc_otg.sc_bus, NULL);

	return (0);
}

static device_method_t dwc_otg_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, dwc_otg_probe),
	DEVMETHOD(device_attach, dwc_otg_attach),
	DEVMETHOD(device_detach, dwc_otg_detach),
	DEVMETHOD(device_suspend, bus_generic_suspend),
	DEVMETHOD(device_resume, bus_generic_resume),
	DEVMETHOD(device_shutdown, bus_generic_shutdown),

	DEVMETHOD_END
};

static driver_t dwc_otg_driver = {
	.name = "dwcotg",
	.methods = dwc_otg_methods,
	.size = sizeof(struct dwc_otg_super_softc),
};

static devclass_t dwc_otg_devclass;

DRIVER_MODULE(dwcotg, root, dwc_otg_driver, dwc_otg_devclass, 0, 0);
MODULE_DEPEND(dwcotg, usb, 1, 1, 1);
