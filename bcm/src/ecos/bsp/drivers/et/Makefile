#
# Broadcom et driver Makefile for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-05-26 14:55:38 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/et/sys -I$(BSPDIR)/drivers/pci -I.
EXTRA_CFLAGS += -DBCM47XX_CHOPS -DETROBO -DCFG_GMAC
#EXTRA_CFLAGS += -DDMA -Werror
EXTRA_CFLAGS += -DDMA

#
# Enumerate files to compile
#
VPATH = $(SRCBASE)/shared:$(SRCBASE)/et/sys

ALL_OBJS = etc.o etc47xx.o etcgmac.o bcmrobo.o et_ecos_common.o et_ecos.o

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)
