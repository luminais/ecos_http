#
# ecos router utilities Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-09-15 12:13:15 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/.config

include $(TOPDIR)/rules.mak

#
# CFLAG propogats to subdirectory
#
#EXTRA_CFLAGS = -Werror

#
# Enumerate subdirectories
#
SUBDIRS = wl et

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#prebuild
PREBUILDDIRS = wl et

prebuilddirs: $(patsubst %, _prebuild_%, $(PREBUILDDIRS))
$(patsubst %, _prebuild_%, $(PREBUILDDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _prebuild_%, %, $@) prebuild

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS)

#prebuild
prebuild: prebuilddirs

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
