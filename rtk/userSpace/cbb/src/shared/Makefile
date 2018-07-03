#
# ecos router shared functions Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-09-15 10:59:36 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak
include $(TOPDIR)/.config

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -DBCMDMA64 -DBCMDBG_ERR
EXTRA_CFLAGS += -Wall -I$(SRCBASE)/router/shared
EXTRA_CFLAGS += -DBCMWPA2

#
# Enumerate files to compile
#
VPATH = $(shell pwd):$(SRCBASE)/router/shared

#ALL_OBJS = wl.o wl_ecos.o wlif_utils.o shutils.o
ALL_OBJS = wl.o wl_ecos.o wlif_utils.o shutils.o nvparse.o defaults.o

#
# Leave rule to make
#
#add by tenda
init_cfg:
	cp -f $(SRCBASE)/ecos/router/config/cfg/$(CONFIG_Tenda_DEFAULT_PROFILE) $(SRCBASE)/router/shared/defaults.c

$(MODULE):  init_cfg $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
