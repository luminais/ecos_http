#
# Broadcom wl driver generic Makefile for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: wl_generic.mk,v 1.1 2010-08-20 02:38:46 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/wl/sys -I$(SRCBASE)/wl/phy -I$(SRCBASE)/include -I$(SRCBASE)/router/shared -I.
EXTRA_CFLAGS += -Wall -I$(BSPDIR)/drivers/pci
EXTRA_CFLAGS += $(WLFLAGS)

#
# Enumerate files to compile
#
VPATH := $(SRCBASE)/wl/sys:$(SRCBASE)/wl/phy:$(SRCBASE)/shared:$(SRCBASE)/bcmcrypto:$(SRCBASE)/router/shared

ifeq ($(strip $(CONFIG_WL_CONF)),)
$(error CONFIG_WL_CONF is undefined)
endif

WLCFGDIR := $(SRCBASE)/wl/config
WLCFGFILE := $(shell ls $(WLCFGDIR)/$(CONFIG_WL_CONF))
include $(WLCFGFILE)
include $(WLCFGDIR)/wl.mk

ifeq ($(strip $(WLFILES)),)
$(error WLFILES is undefined)
endif

WLFILES += wl_ecos_comm.c
INDIRECT_SOURCES := $(WLFILES)

INDIRECT_SOURCES += wl_ecos.c

ALL_OBJS := $(INDIRECT_SOURCES:.c=.o)

UPDATESH   := $(WLCFGDIR)/diffupdate.sh
WLTUNEFILE ?= wltunable_ecos_router.h

#
# Leave rule to make
#
bcmwpa.o : EXTRA_CFLAGS += -DBCMSUP_PSK

wlconf.h: $(WLCFGDIR)/$(WLTUNEFILE) FORCE
	[ ! -f $@ ] || chmod +w $@
	@echo "check and update config file"
	@echo $(if $(VLIST),"VLIST          = $(VLIST)")
	@echo "CONFIG_WL_CONF = $(CONFIG_WL_CONF)"
	@echo "WLTUNEFILE     = $(WLTUNEFILE)"
	cp $< wltemp
	$(UPDATESH) wltemp $@
FORCE:
