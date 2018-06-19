#
# ecos bsp net Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.13 2010-08-09 05:32:06 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TOPDIR)/.config
include $(BSPDIR)/.config

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# CFLAG propogats to subdirectory
#
EXTRA_CFLAGS = -Werror -I$(shell pwd)/include

#
# Enumerate subdirectories
#

SUBDIRS = ipdev kdev keapol

ifeq ($(CONFIG_NAT),y)
SUBDIRS += ipfilter
endif
ifeq ($(CONFIG_DHCPC),y)
SUBDIRS += kdhcpc
endif
ifeq ($(CONFIG_DHCPD),y)
SUBDIRS += kdhcpd
endif
ifeq ($(CONFIG_PPPOE),y)
SUBDIRS += pppoe
endif
ifeq ($(CONFIG_PPTP),y)
SUBDIRS += pptp
endif
ifeq ($(CONFIG_L2TP),y)
SUBDIRS += l2tp
endif
ifeq ($(CONFIG_PPP),y)
SUBDIRS += ppp
endif
ifeq ($(CONFIG_BATCH_UPGRADE),y)
SUBDIRS += batchupgrade
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
