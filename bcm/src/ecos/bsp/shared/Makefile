#
# Shared functions for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.2 2010-05-26 14:55:47 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/include -I$(SRCBASE)/router/shared
EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += -DBCMDRIVER $(WLFLAGS)

#
# Enumerate files to compile
#
VPATH := $(SRCBASE)/shared

WLSHARED := wlconfig_ecos_shared
WLCFGDIR := $(SRCBASE)/wl/config
include $(WLCFGDIR)/$(WLSHARED)
include $(WLCFGDIR)/wl.mk
INDIRECT_SOURCES := $(WLFILES)

INDIRECT_SOURCES += ecos_osl.o bcmallocache.o

OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)
