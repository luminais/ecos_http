#
# ecos router httpd module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.2 2010-05-26 14:23:58 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
	VENDOR:= broadcom
endif

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Enumerate subdirectories
#
SUBDIRS = src lib

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS) www.bin
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS) -bbinary www.bin

$(TOPDIR)/www/$(VENDOR).tar:
	$(MAKE) -C $(TOPDIR)/www

www.bin: $(TOPDIR)/www/$(VENDOR).tar
	cp $(TOPDIR)/www/$(VENDOR).tar ./www.bin

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f
	rm -rf www.bin
	$(MAKE) clean -C $(TOPDIR)/www
	

.PHONY:	clean
