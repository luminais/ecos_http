#
# Broadcom wl_ap driver Makefile for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.2 2010-08-20 10:07:26 Exp $
#

include $(BSPDIR)/.config

CONFIG_WL_CONF := $(CONFIG_WL_AP)

# Include generic make file
include ../wl_generic.mk

#WLTUNEFILE ?= wltunable_router_ecos.h

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): wlconf.h $(ALL_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(filter $(ALL_OBJS), $^)
endif
