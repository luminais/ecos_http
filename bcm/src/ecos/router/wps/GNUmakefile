#
# ecos router wps module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.11 2010-10-13 02:34:49 Exp $
#

# Build type
export BLDTYPE = release
#export BLDTYPE = debug

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TOPDIR)/rules.mak

include $(TOPDIR)/.config

# sys kernel config
include $(BSPDIR)/.config

export WPSSRC = $(SRCBASE)/wps

# Get wireless driver support mode
ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G)$(CONFIG_WL_USE_APSTA)$(CONFIG_WL_USE_APSTA_ONCHIP_G),)
  ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G),)
    export WL_MODE = sta
    export WPS_CONF_FILE = wpsconfig_router_sta
    WPS_LIB_FILE = wps_sta
  else
    export WL_MODE = apsta
    export WPS_CONF_FILE = wpsconfig_router_apsta
    WPS_LIB_FILE = wps_apsta
  endif
else
  export WL_MODE = ap
  export WPS_CONF_FILE = wpsconfig_router_ap
  WPS_LIB_FILE = wps_ap
endif

# Include wps common make file
include $(WPSSRC)/common/config/$(WPS_CONF_FILE)
ifneq ($(CONFIG_WFI),y)
WPS_WFI = 0
endif
ifneq ($(CONFIG_LIBUPNP),y)
WPS_UPNP_DEVICE = 0
endif
include $(WPSSRC)/common/config/wps.mk
EXTRA_CFLAGS = $(WPS_FLAGS) ${WPS_INCS}

# Generate list of supported variants
VLIST  = AP STA APSTA

# Set suffix and config based on the selected variant
WPS_SUFFIX      := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))

# Module file names use lowercase of the suffix
wps_suffix      := $(shell echo "$(WPS_SUFFIX)" | tr A-Z a-z)

# Always do variants build
wpssubdirs := $(addprefix wps_,$(shell echo "$(VLIST)" | tr A-Z a-z))

#
# Set CFLAGS
#
CFLAGS += -DWPS_AP_M2D
CFLAGS += -DWPS_ADDCLIENT_WWTP
CFLAGS += -Wall -Wnested-externs -D_REENTRANT -DBCMDRIVER


CFLAGS += -I$(SRCBASE)/include -I$(SRCBASE)/wps/common/include \
	-I$(SRCBASE)/router/libbcm -I$(SRCBASE)/router/eapd \
	-I$(SRCBASE)/router/shared  -I$(SRCBASE)/wps/brcm_apps/include \
	-I$(SRCBASE)/include/bcmcrypto

LDFLAGS = -r
ifeq ($(WPS_UPNP_DEVICE),1)
CFLAGS += -I$(SRCBASE)/router/libupnp/include -I$(SRCBASE)/wps/brcm_apps/upnp/WFADevice \
	  -I$(SRCBASE)/router/libupnp/ecos
endif

WPS_OSL := $(WPS_ROUTERHALFILES)
WPS_OSL += src/wps/brcm_apps/ecos/wps_ecos_main.c

WPS_OSL += src/wps/brcm_apps/ecos/wps_ecos_osl.c
ifneq ($(WL_MODE), ap)
WPS_OSL += src/wps/brcm_apps/ecos/wps_enr_ecos_osl.c
endif

WPS_OSLOBJS := $(foreach file, $(WPS_OSL), \
	 $(patsubst src/%.c,obj/%.o,$(file)))

WPS_DIRS := $(foreach file, $(WPS_OSLOBJS), \
	 $(dir $(file)))

WPS_DIRLIST = $(sort $(WPS_DIRS)) 


ifneq ($(wildcard ./prebuilt),)
all: extract_prebuilt osl $(MODULE)
else
all: sta_build_check dirs variants_build osl $(MODULE)
endif

extract_prebuilt:
ifneq ($(wildcard prebuilt),)
	(cd prebuilt && tar zxf libwps.tgz)
endif

osl:
	mkdir -p $(WPS_DIRLIST)
	@echo "==>$(WPS_CONF_FILE)"

$(WPS_OSLOBJS) : obj%.o: $(addprefix $(SRCBASE)/../,src%.c)
	$(XCC) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $<

# Variants build rules
variants_build: $(patsubst %, _dir_%, $(wpssubdirs))
$(patsubst %, _dir_%, $(wpssubdirs)):
	[ ! -d $(patsubst _dir_%, %, $@) ] || $(MAKE) CFLAGS="$(CFLAGS)" -C $(patsubst _dir_%, %, $@)

# Set link libraries search path
export LIBPATH += -L./lib -L./prebuilt


sta_build_check:
ifeq ($(WL_MODE),sta)
ifeq ($(WPS_UPNP_DEVICE),1)
	@echo "ERROR: UPNP_WFA_DEVICE is not supported on STA mode"
	@exit 1
endif
endif
	@true

dirs :
	mkdir -p lib

# target
# force compile MODULE in case the library changed.
$(MODULE): $(WPS_OSLOBJS) FORCE
	$(XLD) -r $(ENDIAN) $(LIBPATH) $(WPS_OSLOBJS) -l$(WPS_LIB_FILE) -o $@

FORCE:


clean:
	find . -name $(MODULE) | xargs rm -f
	find . -name obj | xargs rm -rf
	rm -rf lib

.PHONY: all clean libs dirs osl
