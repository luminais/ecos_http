#
# wps module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# Description: ecos Makefile of wps module
# Author : hong
# Version: 1.0
# Created Time: Tue 26 Jan CST
#


MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config

#
# Enumerate subdirectories
#
ifeq ($(CONFIG_WPS_RTK),y)
SUBDIRS += rtk
endif

#
# Set CFLAGS or EXTRA_CFLAGS
#
#EXTRA_CFLAGS += -I$(shell pwd)/common
DEBUG_CFLAGS = 
EXTRA_CFLAGS += -I$(TOPDIR)/prod/http/include/
EXTRA_CFLAGS += -D__ECOS -DBCM47XX -D__ECOS -DROUTER_TINY -DBCMWPA2

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#

ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $@ $(SUB_OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
