#
# ecos router auto connect module Makefile
# 
# Copyright (C) 2010, Tenda Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.0 2015/03/13 Exp $
#	$author: hong$
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

#tenda add -I$(BSPDIR)/include for use kautoconf.h

EXTRA_CFLAGS += -I$(BSPDIR)/include -I$(SRCBASE)/wps/brcm_apps/include -I$(shell pwd)/include -I$(WPS_HAL)/include -I$(shell pwd)/../tc
CFLAGS+=-I../nvram/
ALL_OBJS	= auto_conn.o auto_conn_common.o

#EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME 

ifdef CONFIG_CHINA_NET_CLIENT
EXTRA_CFLAGS += -DCONFIG_CHINA_NET_CLIENT
endif
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
