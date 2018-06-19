#
# ecos router pppoecd module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.2 2010-07-19 10:47:05 Exp $
#

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
PPPDIR = $(TOPDIR)/ppp/package
EXTRA_CFLAGS = -I$(shell pwd) -I$(PPPDIR)/pppd -I$(TOPDIR)/include/crypto
EXTRA_CFLAGS += -I$(SRCBASE)/router/shared

#
# Enumerate files to compile
#
ALL_OBJS = pppoe_ecos.o pppoe_mainloop.o pppoe_fsm.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS) 
