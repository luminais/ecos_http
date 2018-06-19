#
# BCM57XX driver Makefile for ecos bsp
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.3 2010-05-26 14:55:38 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/bcm57xx/sys -I$(SRCBASE)/bcm57xx/ecos -I$(BSPDIR)/drivers/pci
EXTRA_CFLAGS += -DDBG=0 -DT3_JUMBO_RCV_RCB_ENTRY_COUNT=256
EXTRA_CFLAGS += -DPCIX_TARGET_WORKAROUND=1 -DINCLUDE_TBI_SUPPORT -DINCLUDE_5701_AX_FIX=1

#
# Enumerate files to compile
#
VPATH = $(SRCBASE)/shared:$(SRCBASE)/bcm57xx/sys:$(SRCBASE)/bcm57xx/ecos

ALL_OBJS = tigon3.o autoneg.o 5701rls.o tcp_seg.o hndgige.o b57um_ecos.o

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)
