#
# ecos router nvram module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.2 2010-05-26 14:24:02 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(SRCBASE)/router/shared
EXTRA_CFLAGS += -Werror

#
# Enumerate subdirectories
#
SUBDIRS = shared

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /%.o, $(SUBDIRS)))

#
# Enumerate files to compile
#
ALL_OBJS = nvram_ecos.o sys_backupcfg.o 

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d) subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS) $(SUB_OBJS)
