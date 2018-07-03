#
# Broadcom web pages
#
# Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: Makefile,v 1.11 2010-07-16 05:46:53 Exp $
#

all: pages.mk


include $(TOPDIR)/.config

# Common pages
PAGES := blur_new.jpg logo_new.gif repeat_edge_new.gif overlib.js style.css

PAGES += index.asp basic.gif 
PAGES += lan.asp lan.gif

ifeq ($(CONFIG_NAT),y)
PAGES += wan.asp wan.gif
endif

# Netboot only has netboot firmware pages
ifeq ($(CONFIG_NETBOOT),y)

PAGES += netboot.asp firmware.gif

else

PAGES += status.asp status.gif
ifeq ($(CONFIG_NAT),y)
PAGES += filter.asp filter.gif
PAGES += forward.asp forward.gif
endif
PAGES += firmware.asp firmware.gif

endif

# Wireless related pages
PAGES += radio.asp radio.gif
PAGES += ssid.asp ssid.gif
PAGES += security.asp security.gif

# Internal pages
ifneq ($(filter -DBCMINTERNAL, $(CFLAGS)),)
PAGES += internal.asp internal.gif
endif

#
# Generate file list
#
ifneq ($(WIND_HOST_TYPE),x86-win32)
pages.mk:
	echo "PAGES := $(PAGES)" > $@
else
pages.mk:
	echo PAGES := $(PAGES) > $@
endif

# clean rule
clean:
	rm -f pages.mk
