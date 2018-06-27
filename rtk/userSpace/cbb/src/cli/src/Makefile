# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co..
#
# Description: ecos Makefile of cli module
# Author : zhuhuan
# Version: 1.0
# Created Time: Tue 19 Jan 2016 14:21:36 PM CST
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


all: $(MODULE)

include $(APP_DIR)/rules.mk
include $(ECOS_WORK_DIR)/.config
include $(USER_CONFIG)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/nvram
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared/
EXTRA_CFLAGS += -I$(TOP_DIR)/ecos-work/AP/shell/
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

# CFLAGS += $(EXTRA_CFLAGS) 
# CXXFLAGS += $(EXTRA_CFLAGS) 

#
# Enumerate files to compile
#
ALL_OBJS = cli_main.o cli_io.o
ALL_OBJS += cli_tenda.o
ALL_OBJS += tenda_arp.o
ALL_OBJS += cli_net.o
ALL_OBJS += cli_basic.o

ifeq ($(CONFIG_WATCH_POINT),y)
ALL_OBJS += cli_wacth_point.o
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
