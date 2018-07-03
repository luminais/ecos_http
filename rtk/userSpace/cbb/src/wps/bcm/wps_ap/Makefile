#
# ecos router wps module for AP Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-10-13 02:35:34 Exp $
#

include $(TOPDIR)/.config

# sys kernel config
include $(BSPDIR)/.config

WL_MODE = ap

# Include generic make file
include ../wps_generic.mk

# Set link libraries search path
LIBPATH += -L../

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(WPSM_OSL_OBJS:.o=.d) $(WPSM_ARCH_OBJS:.o=.d) $(WPSM_UPNP_OBJS:.o=.d) $(LIBWPS_A)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(WPSM_OSL_OBJS) $(WPSM_ARCH_OBJS) $(WPSM_UPNP_OBJS) $(LIBPATH) $(WPSM_LIBS)
endif

#
# 1. Full source no "prebuilt libraries" and obj-wps.o.gz
# 2. Partial source has "prebuilt libraries" only
# 3. Otherwise has obj-wps.o.gz and wps_ap/obj-wps_ap.o.gz and wps_apsta/obj-wps_apsta.o.gz
