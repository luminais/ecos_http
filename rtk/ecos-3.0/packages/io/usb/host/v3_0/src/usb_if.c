/*
 * This file is produced automatically.
 * Do not modify anything in here by hand.
 *
 * Created from source file
 *   usb_if.m
 * with
 *   makeobjops.awk
 *
 * See the source file for legal information
 */

#include <cyg/io/usb_sys/param.h>
#include <cyg/io/usb_sys/queue.h>
#include <cyg/io/usb_sys/kernel.h>
#include <cyg/io/usb_sys/kobj.h>
#include <cyg/io/usb_sys/bus.h>
#include "usb_if.h"

struct kobj_method usb_handle_request_method_default = {
	&usb_handle_request_desc, (kobjop_t) kobj_error_method
};

struct kobjop_desc usb_handle_request_desc = {
	0, &usb_handle_request_method_default
};

