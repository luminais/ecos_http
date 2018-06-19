#
# ecos router libupnp Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-05-26 14:24:00 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

include $(TOPDIR)/.config

include $(TOPDIR)/rules.mak

LIBUPNP = $(SRCBASE)/router/libupnp
SRCPATH = $(LIBUPNP)/upnp

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(LIBUPNP)/include -I$(LIBUPNP)/ecos \
	       -I$(SRCBASE)/include -I$(SRCBASE)/include/bcmcrypto -I$(SRCBASE)/router/shared

#
# Enumerate files to compile
#
vpath %.c $(SRCPATH) $(LIBUPNP)/ecos
vpath %.o $(TOPDIR)/libupnp/prebuilt

ALL_SRCS = upnp.c upnp_ssdp.c upnp_http.c upnp_gena.c upnp_soap.c \
	   upnp_description.c upnp_device.c upnp_util.c upnp_msg.c \
	   upnp_ecos_osl.c

ALL_OBJS = ${ALL_SRCS:.c=.o}

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
