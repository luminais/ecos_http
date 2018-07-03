#
# ecos router wps module generic Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: wps_generic.mk,v 1.6 2011-02-11 02:45:03 Exp $
#

include $(TOPDIR)/rules.mak

# Include wps common make file
include	$(WPSSRC)/common/config/$(WPS_CONF_FILE)
ifneq ($(CONFIG_WFI),y)
WPS_WFI = 0
endif
ifneq ($(CONFIG_LIBUPNP),y)
WPS_UPNP_DEVICE = 0
endif
include $(WPSSRC)/common/config/wps.mk
EXTRA_CFLAGS = ${WPS_INCS} ${WPS_FLAGS}


WPS_SOURCE := $(WPS_FILES)

# Add bcmgpio
WPS_SOURCE += src/router/libbcm/bcmgpio.c

vpath %.c $(SRCBASE)/../

WPS_OBJS := $(foreach file, $(WPS_SOURCE), \
	  $(patsubst src/%.c, obj/%.o,$(file)))

WPS_DIRS   := $(foreach file, $(WPS_OBJS), \
	$(dir $(file)))

WPS_DIRLIST = $(sort $(WPS_DIRS)) 


all: dirs $(LIB)

dirs:
	mkdir -p $(WPS_DIRLIST)
	@echo "==> $(WPS_CONF_FILE)"

# library rules
$(LIB) : $(WPS_OBJS)
	$(XAR) cr $@ $(WPS_OBJS)

$(WPS_OBJS) : obj%.o: $(addprefix $(SRCBASE)/../,src%.c)
	$(XCC) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $<

clean:
	rm -rf obj
