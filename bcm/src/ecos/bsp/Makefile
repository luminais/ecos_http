#
# ecos bsp Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.17 2010-08-05 08:10:26 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ARCH = mips

# sys .config
include .config

ifeq ("$(SRCBASE)","")
export SRCBASE = $(shell cd ../.. ; pwd)
export ECOS_REPOSITORY=$(SRCBASE)/ecos/ecos-3.0/packages
export ECOS_TOOLS=$(SRCBASE)/ecos/ecos-3.0/tools/bin
endif

ifeq ("$(BSPDIR)","")
export BSPDIR = $(shell pwd)
endif

all: epivers $(MODULE)

include $(BSPDIR)/rules.mak

#
# Local Make EXTRA_CFLAGS and Command
#
# Include other rules
include $(BSPDIR)/kernel/ecos_install/include/pkgconf/ecos.mak

ifeq ("$(TOPDIR)","")
export CROSS_PREFIX = $(ECOS_COMMAND_PREFIX)
export ENDIAN = $(shell echo $(ECOS_GLOBAL_CFLAGS) | sed -e "s/.*-EL.*/-EL/" )
endif

#EXTRA_CFLAGS = -I$(BSPDIR)
EXTRA_CFLAGS += -I$(BSPDIR)/include -I$(BSPDIR)/kernel/ecos_install/include -I$(SRCBASE)/include -I$(SRCBASE)/router/shared
EXTRA_CFLAGS += -D__ECOS $(ECOS_GLOBAL_CFLAGS) -include kconfig.h
EXTRA_CFLAGS += -D_KERNEL -D__INSIDE_NET
EXTRA_CFLAGS += -DBCMDMA32

#
# Configuration rules
#
.config:
	cp arch/mips/defconfig-bcm947xx $@
	make oldconfig

oldconfig:
	bash $(BSPDIR)/scripts/Menuconfig $(BSPDIR)/arch/$(ARCH)/config.in save
	if [ -e $(SRCBASE)/wl/config/diffupdate.sh ] ; then \
		chmod u+rx $(SRCBASE)/wl/config/diffupdate.sh ; \
	fi

menuconfig:
	bash $(BSPDIR)/scripts/Menuconfig $(BSPDIR)/arch/$(ARCH)/config.in

mrproper: clean
	rm -f .config .config.old .menuconfig.log .version
	rm -f $(BSPDIR)/drivers/wl/wlconf.h
	rm -f $(BSPDIR)/include/kautoconf.h
	rm -rf kernel

epivers:
	if [ ! -f $(SRCBASE)/include/epivers.h ]; then \
		make -C $(SRCBASE)/include ; \
	fi

$(BSPDIR)/kernel/ecos_install/include/pkgconf/ecos.mak: kernel

kernel:	.config
	@echo "Build eCos kernel library"
	if [ ! -d $(BSPDIR)/kernel ] ; then \
		mkdir -p $(BSPDIR)/kernel ; \
	fi
	if [ ! -e $(BSPDIR)/kernel/ecos.ecc ] ; then \
		cp -f $(BSPDIR)/arch/$(ARCH)/$(CONFIG_ECC) $(BSPDIR)/kernel/ecos.ecc ; \
	fi
	if [ ! -d $(BSPDIR)/kernel/ecos_install ] ; then \
		cp -f $(BSPDIR)/kernel/ecos.ecc $(BSPDIR)/kernel/ecos_work.ecc ; \
		mkdir -p $(BSPDIR)/kernel/ecos_build ; \
		cd $(BSPDIR)/kernel/ecos_build ; \
		$(ECOS_TOOLS)/ecosconfig --config=$(BSPDIR)/kernel/ecos_work.ecc --prefix=$(BSPDIR)/kernel/ecos_install tree ; \
		make -C $(BSPDIR)/kernel/ecos_build ; \
		rm -f $(BSPDIR)/kernel/ecos_work.ecc ; \
	fi \

#
# Enumerate subdirectories
#
SUBDIRS = init drivers shared net
#SUBDIRS += ipfilter

include arch/$(ARCH)/Makefile

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(notdir $(SUBDIRS))))

# sys prebuild
PREBUILTDIRS = drivers/et drivers/wl
bsp-prebuild: $(patsubst %, _prebuild_%, $(PREBUILTDIRS))
$(patsubst %, _prebuild_%, $(PREBUILTDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _prebuild_%, %, $@) prebuild

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS)

clean:
	find $(SUBDIRS) -name '*.[od]' -type f -print | xargs rm -f
	rm -f $(MODULE)
	rm -f drivers/wl/wlconf.h

prebuild: bsp-prebuild

.PHONY: clean
