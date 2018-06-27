#
# auto_conn_client module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# Description: ecos Makefile of auto_conn_client module
# Author : hong
# Version: 1.0
# Created Time: Tue 26 Jan 2016 CST
#


MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)
include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#
#EXTRA_CFLAGS = -I. -I$(shell cd ../ ; pwd)/common
#EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/kdhcpc
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/nvram
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/lib/cJSON/
EXTRA_CFLAGS +=-D_KERNEL -DBCM47XX -D__ECOS -DROUTER_TINY -DCFG_GMAC -DBCMWPA2

ifeq ($(CONFIG_EXTEND_LED),y)
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/system/include
endif
#
# Enumerate files to compile
#
ALL_OBJS = auto_conn_common.o

ifeq ($(CONFIG_AUTO_CONN_CLIENT),y)
ALL_OBJS += auto_conn_client_main.o
endif

ifeq ($(CONFIG_AUTO_CONN_SERVER),y)
ALL_OBJS += auto_conn_server_main.o
endif

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
