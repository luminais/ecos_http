#
# ecos router dnsmasq module Makefile
# 
# Copyright (C) 2009, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.3 2010/07/26 01:44:42 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


all: $(MODULE)

include $(ECOS_AP_DIR)/rules.mk
include $(ECOS_WORK_DIR)/.config

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I$(shell cd .. ; pwd) -I$(NETDRV_DIR)/kdev

ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

# CFLAGS += $(EXTRA_CFLAGS) 
# CXXFLAGS += $(EXTRA_CFLAGS) 

#
# Enumerate files to compile
#
ALL_OBJS = ip_dev.o

#
# Leave rule to make
#

#$(MODULE): $(ALL_OBJS:.o=.d)
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
