#
# ecos router ppp module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-07-02 04:32:42 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

# router config
include $(TOPDIR)/.config

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# EXTRA_CFLAG propogats to subdirectory
#
#EXTRA_CFLAGS = -Werror -I$(BSPDIR)/net/include
#roy modify
EXTRA_CFLAGS =  -I$(BSPDIR)/net/include

#
# Enumerate subdirectories
#
obj-y = package

subdirs: $(patsubst %, _dir_%, $(obj-y))
$(patsubst %, _dir_%, $(obj-y)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

subobjs = $(join $(obj-y), $(patsubst %, /obj-%.o, $(obj-y)))

#
# Node rule to make
#
$(MODULE): subdirs $(subobjs)
	$(XLD) -r $(ENDIAN) -o $@ $(subobjs)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
