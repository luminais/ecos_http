#
# Kernel pppoe client device Makefile for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-07-05 10:35:29 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(BSPDIR)/.config

all: $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(BSPDIR)/net/kdev -I$(BSPDIR)/include/crypto

#
# Enumerate files to compile
#
PPPOE_OBJS = ifpppoe_usrreq.o

ALL_OBJS = $(PPPOE_OBJS) $(PPPOE_MD_OBJS)

MODULE_OBJS = $(PPPOE_OBJS)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(MODULE_OBJS) 
