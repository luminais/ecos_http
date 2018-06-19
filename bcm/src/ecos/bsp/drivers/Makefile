#
# ecos bsp drivers Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.3 2010-05-26 14:55:37 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(BSPDIR)/.config

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# CFLAG propogats to subdirectory
#
EXTRA_CFLAGS += -DBCMDRIVER

#
# Enumerate subdirectories
#

SUBDIRS = flash

ifdef CONFIG_ET_47XX
SUBDIRS += et
endif

ifdef CONFIG_BCM57XX
SUBDIRS += bcm57xx
endif

ifdef CONFIG_WL
SUBDIRS += wl
endif

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS)
