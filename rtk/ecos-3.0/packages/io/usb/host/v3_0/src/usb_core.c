/* $FreeBSD$ */
/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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

/*
 * USB specifications and other documentation can be found at
 * http://www.usb.org/developers/docs/ and
 * http://www.usb.org/developers/devclass_docs/
 */


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

#include <cyg/io/usb_dev/usb/usb.h>
#include <cyg/io/usb_dev/usb/usbdi.h>



MALLOC_DEFINE(M_USB, "USB", "USB");
MALLOC_DEFINE(M_USBDEV, "USBdev", "USB device");
MALLOC_DEFINE(M_USBHC, "USBHC", "USB host controller");
MALLOC_DEFINE(M_TEMP, "temp", "misc temporary data buffers");


MODULE_VERSION(usb, 1);
