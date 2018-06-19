#
# ecos router root config Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.17 2010-07-24 12:50:36 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

# router config file
include $(TOPDIR)/.config

# sys config file
include $(BSPDIR)/.config

all: $(MODULE)
include $(TOPDIR)/rules.mak

#VPATH = $(ARCH_DIR)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I. -Werror   -I$(shell cd .. ; pwd)/batchupgrade/ 
CFLAGS+=-I../nvram/
#
# Enumerate files to compile
#
# include .config if any compile option
ALL_OBJS = batchupgrade.o upgrade.o
# ALL_OBJS += interface.o sntpc.o dns.o

#
# Recursively to next directory
#
SUBDIRS = 

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /%.o, $(SUBDIRS)))

ALL_OBJS += $(SUB_OBJS)

#
# Leave rule to make
#
$(MODULE): subdirs $(ALL_OBJS) $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f

.PHONY:	clean