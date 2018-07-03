#
#  ecos router telnetd module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
# 
# $Id:
# 

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE) $(ALL_OBJS) $(ALL_OBJS:.o=.d)

include $(TOPDIR)/rules.mak

TELNETD_SRC_DIR = $(SRCBASE)/router/telnetd
CUR_DIR = $(shell pwd)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(EAPD_SRC_DIR) -I$(SRCBASE)/shared -I$(SRCBASE)/include -I$(SRCBASE)/router/shared -I$(CUR_DIR)/include
EXTRA_CFLAGS += -Wall -Werror
#EXTRA_CFLAGS += -DLINEMODE

#
# Enumerate files to compile
#
VPATH = $(TELNETD_SRC_DIR)

INDIRECT_SOURCES = telnetd_ecos.c telnetd.c state.c global.c sys_term.c utility.c slc.c termstat.c telnetd_monitor.c


ALL_OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY: clean
