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

include $(TOPDIR)/.config

# sys kernel config
include $(BSPDIR)/.config

all : cp_variant

# Generate list of supported variants
VLIST_BASE     ?= AP STA APSTA

VLIST          += $(VLIST_BASE)

ifeq ($(CONFIG_WL_USBAP),y)
# Do variants because we need two drivers for dual band operation
WPS_VARIANTS   := 1
VLIST          += HIGH
# Note that we don't use CONFIG_WL_USE_HIGH because we need to leave WL_SUFFIX for
# the other driver. HIGH driver is always called wl_high and loaded upon hotplut
# event.
endif

ifeq ($(BUILD_WL_ONCHIP_G),true)
  VLIST        += $(foreach base,$(VLIST_BASE),$(base)_ONCHIP_G)
endif # BUILD_WL_ONCHIP_G

# Set suffix and config based on the selected variant
WL_SUFFIX      := $(strip $(foreach v,$(VLIST),$(if $(CONFIG_WL_USE_$(v)),$(v))))

# Module file names use lowercase of the suffix
wl_suffix      := $(shell echo "$(WL_SUFFIX)" | tr A-Z a-z)

# Get wireless driver support mode
ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G)$(CONFIG_WL_USE_APSTA)$(CONFIG_WL_USE_APSTA_ONCHIP_G),)
  ifneq ($(CONFIG_WL_USE_STA)$(CONFIG_WL_USE_STA_ONCHIP_G),)
    WL_MODE = sta
    UPNP_WFA_DEVICE = 0
  else
    WL_MODE = apsta
  endif
else
  WL_MODE = ap
endif

# Include generic make file
include ./wps_generic.mk

#
# Leave rule to make
#


# Copy variant prebuild to target (!FULL && !ROUTERSRC)
cp_variant:
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
ifneq ($(wildcard prebuilt),)
	gzip -d -c prebuilt/libwps_$(wl_suffix).a.gz > libwps_$(wl_suffix).a
else
	@echo "Ignore copy variant..."
endif
else
	cp wps_$(wl_suffix)/obj-wps_$(wl_suffix).o.gz ./$(MODULE).gz
endif

# Set link libraries search path
export LIBPATH += -L. -L./prebuilt

all: sta_build_check

sta_build_check:
ifeq ($(WL_MODE),sta)
ifeq ($(UPNP_WFA_DEVICE),1)
	@echo "ERROR: UPNP_WFA_DEVICE is not supported on STA mode"
	@exit 1
endif
endif
	@true

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
ifneq ($(wildcard prebuilt),prebuilt)
$(MODULE): $(LIBWPS_A)
endif
$(MODULE): $(WPSM_OSL_OBJS:.o=.d) $(WPSM_ARCH_OBJS:.o=.d) $(WPSM_UPNP_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(WPSM_OSL_OBJS) $(WPSM_ARCH_OBJS) $(WPSM_UPNP_OBJS) $(LIBPATH) $(WPSM_LIBS)
endif

#
# 1. Full source no "prebuilt libraries" and obj-wps.o.gz
# 2. Partial source has "prebuilt libraries" only
# 3. Otherwise has obj-wps.o.gz and wps_ap/obj-wps_ap.o.gz and wps_apsta/obj-wps_apsta.o.gz

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f

.PHONY: all clean
