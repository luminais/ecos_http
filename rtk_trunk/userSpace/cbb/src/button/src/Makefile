# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, 
# copied or duplicated in any form, in whole or in part, without 
# the prior written permission of Tenda Tech. Co..
#
# Description: ecos Makefile of button module
# Author : zhuhuan
# Version: 1.0
# Created Time: Mon Jul  4 10:29:59 CST 2016
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


all: $(MODULE)

include $(APP_DIR)/rules.mk
include $(TENDA_APP_DIR)/.config

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/nvram
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/gpio_api/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/system/include/
# EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk/
# EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared/

ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

# CFLAGS += $(EXTRA_CFLAGS) 
# CXXFLAGS += $(EXTRA_CFLAGS) 

#
# Enumerate files to compile
#
ALL_OBJS = common.o
ALL_OBJS += reset_button.o
ifeq ($(CONFIG_WIFI_BUTTON_RTK),y)
ALL_OBJS += wifi_button.o
endif
ifeq ($(CONFIG_WPS_RTK),y)
ALL_OBJS += wps_button.o
endif


#
# Leave rule to make
# 

#$(MODULE): $(ALL_OBJS:.o=.d)
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
