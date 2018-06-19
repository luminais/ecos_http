#
# Broadcom eCos Router Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.24 2010-12-17 02:29:55 Exp $
#

include .config

#add by 6091@tenda.cn
ECOS_MIPSTOOL_PATH = /home/work/2016/BCM/toolchains/ecos/gnutools/mipsisa32-elf/bin
ECOS_BCM_TOOL_PATH = $(shell cd ../../../tools ; pwd)
PATH :=$(ECOS_MIPSTOOL_PATH):$(ECOS_BCM_TOOL_PATH):$(PATH)
PROFILE ?= 

export PATH TOPDIR ENDIAN CROSS_PREFIX SRCBASE BSPDIR VENDOR CFLAGS ECOS_REPOSITORY ECOS_TOOLS IPF_PKG PROFILE


#export TOPDIR ENDIAN CROSS_PREFIX SRCBASE BSPDIR VENDOR CFLAGS ECOS_REPOSITORY ECOS_TOOLS IPF_PKG

# Set SRCBASE
SRCBASE=$(shell cd ../.. ; pwd)

# Set BSPDIR

BSPDIR=$(shell cd ../bsp ; pwd)

# Include other rules
include $(BSPDIR)/kernel/ecos_install/include/pkgconf/ecos.mak

include rules.mak

ECOS_REPOSITORY=$(SRCBASE)/ecos/ecos-3.0/packages
ECOS_TOOLS=$(SRCBASE)/ecos/ecos-3.0/tools/bin

IPF_PKG = ip_fil3.4.35

#
# Set CFLAGS or EXTRA_CFLAGS
#
CROSS_PREFIX=$(ECOS_COMMAND_PREFIX)
ENDIAN=$(shell echo $(ECOS_GLOBAL_CFLAGS) | sed -e "s/.*-EL.*/-EL/" )

TOPDIR=$(shell pwd)
VENDOR:= broadcom

CFLAGS = -I$(TOPDIR)
CFLAGS += -I$(TOPDIR)/include -I$(BSPDIR)/include -I$(BSPDIR)/kernel/ecos_install/include -I$(SRCBASE)/include -I$(SRCBASE)/router/shared
CFLAGS += -I$(SRCBASE)/shared/zlib

CXXFLAGS = $(CFLAGS)
LDFLAGS = -nostartfiles -L$(BSPDIR)/kernel/ecos_install/lib -T$(BSPDIR)/kernel/ecos_install/lib/target.ld

CFLAGS += -D__ECOS $(ECOS_GLOBAL_CFLAGS)
CFLAGS += -Wno-strict-prototypes -Wno-pointer-sign

CFLAGS += -DCFG_GMAC
CFLAGS += -include config.h

# always has BCMWPA2
CFLAGS += -DBCMWPA2

#
# Enumerate subdirectories
#

#
# Setup directory with no KERNEL flag
#

# always build objects
obj-y = rc cli lib nvram shared bcmcrypto 
obj-y += utils 
obj-y += nas eapd wlconf

ifneq ($(CONFIG_DHCPC)$(CONFIG_DHCPD),)
obj-y += bcmdhcpd
endif

obj-$(CONFIG_NAT) +=ipf
obj-y += libz
obj-$(CONFIG_HTTPD) += httpd
obj-$(CONFIG_PPP) += ppp
obj-$(CONFIG_DNSMASQ) += dnsmasq
obj-$(CONFIG_NETBOOT) += netboot
obj-$(CONFIG_LIBUPNP) += libupnp
obj-$(CONFIG_IGD) += igd
obj-$(CONFIG_WPS) += wps
obj-$(CONFIG_TELNETD) += telnetd 
#add by 6091@tenda.cn
obj-$(CONFIG_DDNS) += ddns
obj-$(CONFIG_8021X) += 8021x
obj-$(CONFIG_TENDA_HTTPD_UCD) += tenda_httpd_ucd
obj-$(CONFIG_TENDA_HTTPD_V3) += tenda_httpd_v3
obj-$(CONFIG_TENDA_HTTPD_NORMAL) += tenda_httpd
obj-$(CONFIG_TC) += tc
obj-$(CONFIG_BATCH_UPGRADE) += batchupgrade
obj-$(CONFIG_ALINKGW) += alinkgw
obj-$(CONFIG_AUTO_CONN) += auto_conn
obj-$(CONFIG_NEW_INFRASTRUCTURE) += infrastructure

ifeq ($(CONFIG_WPS),y)
CFLAGS += -D__CONFIG_WPS__
# WFA WPS 2.0 Testbed extra caps
#CFLAGS += -DWFA_WPS_20_TESTBED
endif

ifeq ($(CONFIG_WEB_DIR),"tenda-ucd-multi")
CFLAGS += -D__CONFIG_QUICK_SET__
endif

obj-clean := $(foreach obj,$(obj-y) $(obj-n),$(obj)-clean)
obj-prebuild := $(foreach obj,$(obj-y) $(obj-n),$(obj)-prebuild)

# separate the libraries which need to be built first
obj-prelibs = $(filter bcmcrypto nas eapd wlconf utils, $(obj-y)) 
# remaining libraries that are built next
obj-postlibs := $(filter-out $(obj-prelibs), $(obj-y))

#
# Basic rules
#

all: epivers version $(BSPDIR)/.config bsp $(obj-prelibs) $(obj-postlibs) ecos.trx ecos_lzma.trx cptftp

post_preplibs: $(obj-postlibs)

#
# router prebuild
#
router_prebuild_dirs = $(filter bcmcrypto nas eapd wlconf utils netboot libupnp igd wps, $(obj-y))

router-prebuild: $(foreach obj,$(router_prebuild_dirs),$(obj)-prebuild)

#
# BSP directory with KERNEL flag
#
bsp:
	$(MAKE) -C $(BSPDIR)

bsp-clean:
	$(MAKE) -C $(BSPDIR) clean
#roy+++,2010/09/28	
	$(RM) -fr $(BSPDIR)/kernel
	$(RM) -fr $(BSPDIR)/include/netinet
	
# bsp prebuild
bsp-prebuild:
	$(MAKE) -C $(BSPDIR) prebuild

ifeq ($(CONFIG_UTILS),y)
TRX_DIRS = $(obj-y)
else
TRX_DIRS = $(filter-out utils, $(obj-y))
endif

TRX_OBJS = $(join $(TRX_DIRS), $(patsubst %, /obj-%.o, $(TRX_DIRS)))
TRX_OBJS += $(BSPDIR)/obj-bsp.o

TRX_LIBS = $(BSPDIR)/kernel/ecos_install/lib/libtarget.a

ecos: version bsp $(TRX_LIBS) $(TRX_OBJS) $(BSPDIR)/kernel/ecos_install/lib/target.ld
	$(XCC) $(LDFLAGS) $(ECOS_GLOBAL_LDFLAGS) -o $@ $(TRX_OBJS) $(TRX_LIBS)

ecos.gz: ecos.bin ecos.map
	gzip -c9 ecos.bin > ecos.gz

ecos.lz: ecos.bin ecos.map
	lzma e ecos.bin ecos.lz


#
# Specially taking care of netboot
#
ifeq ($(CONFIG_NETBOOT),y)
zLoader:
	$(MAKE) -C zloader

ifeq ($(CONFIG_WEB_VERSION),"cn")

ifeq ($(CONFIG_TENDA_HTTPD_NORMAL),y)
ecos.trx: ecos.gz
	trx -o ecos.trx ecos.gz

ecos_lzma.trx: ecos.lz
	trx -o ecos_lzma.trx ecos.lz
	
else
ecos.trx: ecos.gz
	trx -f 0x0080 -o ecos.trx ecos.gz
ecos_lzma.trx: ecos.lz
	trx -f 0x0080 -o ecos_lzma.trx ecos.lz	
endif

else 

ecos.trx: ecos.gz zLoader
	trx -o ecos.trx ecos.gz zloader/zLoader
	
ecos_lzma.trx: ecos.lz zLoader
	trx -o ecos_lzma.trx ecos.lz zloader/zLoader

endif

	
else

ifeq ($(CONFIG_WEB_VERSION),"cn")
	
ifeq ($(CONFIG_TENDA_HTTPD_NORMAL),y)
ecos.trx: ecos.gz
	trx -o ecos.trx ecos.gz

ecos_lzma.trx: ecos.lz
	trx -o ecos_lzma.trx ecos.lz
	
else
ecos.trx: ecos.gz
	trx -f 0x0080 -o ecos.trx ecos.gz
ecos_lzma.trx: ecos.lz
	trx -f 0x0080 -o ecos_lzma.trx ecos.lz	
endif
	
else 

ecos.trx: ecos.gz
	trx -o ecos.trx ecos.gz

ecos_lzma.trx: ecos.lz
	trx -o ecos_lzma.trx ecos.lz

endif

	
endif

cptftp: ecos.bin ecos.trx
	-cp ecos.trx /home/tftpboot
	-cp ecos_lzma.trx /home/tftpboot
# pxy add, for debug
	objdump -S ecos > ecos_assemb
router-clean: www-clean
	@echo "clean all.."
	rm -f ecos.trx ecos_lzma.trx ecos.gz ecos.lz ecos.bin ecos.map ecos.sizes ecos ecos_assemb.txt
	find . \( -name '*.[oasd]' ! -name obj-alinkgw_sdk.o ! -name obj-asec.o ! -name obj-cyassl.o ! -name obj-json-c.o ! -name oobj-src.o -o -name '*.tmp' -o -name '*.bak' -o -name '*~' -o -name core -o -name '.*.flags' \) -type f -print | xargs rm -f
ifeq ($(CONFIG_NETBOOT),y)
	make -C zloader clean
endif

www-clean:
	@echo "clean www.."
	rm -rf $(TOPDIR)/www/$(VENDOR).tar
	rm -rf $(TOPDIR)/httpd/www.bin
	make -C www clean

clean: router-clean bsp-clean

#
# Clean all object files in the VPATH
#
obj-vclean := bcmcrypto et router shared wl wps

%-vclean:
	find $(SRCBASE)/$* -name '*.[od]' -type f -print | xargs rm -f

vclean: $(foreach obj,$(obj-vclean),$(obj)-vclean)
	rm -f $(SRCBASE)/router/shared/bcmconfig.h

#
# Configuration rules
#
oldconfig: vclean before_oldconfig koldconfig
	chmod u+rx scripts/lxdialog/lxdialog
	bash scripts/Menuconfig config/config.in save
	cp include/autoconf.h $(SRCBASE)/router/shared/bcmconfig.h
	cp include/autoconf.h $(SRCBASE)/ecos/ecos-3.0/packages/net/bsd_tcpip/v3_0/include/netinet/tendaconfig.h

menuconfig:
	bash scripts/Menuconfig config/config.in
	make k$@
	
before_oldconfig:
	@if [ ! -f $(BSPDIR)/config_$(PROFILE) ] ; then \
		echo "===================================================================" ; \
        	echo "     Wrong Firmware: "$(PROFILE)                              ; \
		echo "     You should run make PROFILE=8MB_AP/8MB_AP_B0/16MB/A5V1/A5V2"       ; \
        	echo "===================================================================" ; \
		exit 1 ; \
	fi	
	@echo "*********************************************"
	@echo "     Firmware Type: "$(PROFILE)"      "
	@echo "*********************************************"
	cp -f $(BSPDIR)/config_$(PROFILE) $(BSPDIR)/.config

koldconfig: $(BSPDIR)/.config
	make -C $(BSPDIR) oldconfig

kmenuconfig: $(BSPDIR)/.config
	make -C $(BSPDIR) menuconfig


# Default configurations
#
.config:
	cp config/defconfig $@
	make oldconfig

$(BSPDIR)/.config:
	make -C $(BSPDIR) .config

mrproper: clean
	rm -f .config .config.old .menuconfig.log .version
	rm -f $(TOPDIR)/include/autoconf.h $(TOPDIR)/include/build_time.h
	rm -f ChangeLog
	rm -f $(SRCBASE)/include/epivers.h
	rm -f $(SRCBASE)/router/shared/router_version.h
	rm -f $(SRCBASE)/wps/common/include/wps_version.h

changelog:
	@echo "Generating ChangeLog"
	if [ -f scripts/cvs2cl.pl ] ; then \
		cd $(SRCBASE) ; \
		chmod +x $(TOPDIR)/scripts/cvs2cl.pl ; \
		$(TOPDIR)/scripts/cvs2cl.pl ; \
		mv $(SRCBASE)/ChangeLog $(TOPDIR)/ ; \
	fi

$(BSPDIR)/kernel/ecos_install/lib/target.ld: kernel

$(BSPDIR)/kernel/ecos_install/include/pkgconf/ecos.mak: .config $(BSPDIR)/.config
	if [ ! -e $(BSPDIR)/kernel/ecos_install/include/pkgconf/ecos.mak ] ; then \
		make -C $(BSPDIR) kernel ; \
	fi

epivers:
	if [ ! -f $(SRCBASE)/include/epivers.h ]; then \
		make -C $(SRCBASE)/include ; \
	fi

version:  $(SRCBASE)/router/shared/router_version.h

# this could fail if the bom doesn't exist. We don't care as long as there is a valid router_version.h
# if not, the build will fail anyway.
$(SRCBASE)/router/shared/router_version.h: $(SRCBASE)/router/shared/version.h.in
	[ ! -e $(SRCBASE)/tools/release/ecos-router-bom.mk  ] ||  make SRCBASE=$(SRCBASE) -f $(SRCBASE)/tools/release/ecos-router-bom.mk version

#
# Generic rules
#

%:
	[ ! -d $* ] || $(MAKE) -C $*

%-clean:
	[ ! -d $* ] || $(MAKE) -C $* clean

%-prebuild:
	[ ! -d $* ] || $(MAKE) -C $* prebuild

$(obj-y) $(obj-n) $(obj-clean) $(obj-prebuild) : dummy

# prebuild
prebuild: router-prebuild bsp-prebuild

.PHONY: all clean dummy
