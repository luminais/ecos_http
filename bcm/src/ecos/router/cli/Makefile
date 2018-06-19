#
# ecos router cli module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.3 2010-05-26 14:23:55 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(BSPDIR)/net/include -I$(BSPDIR)/ipfirewall/include

#
# Enumerate subdirectories
#
SUBDIRS = src

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
