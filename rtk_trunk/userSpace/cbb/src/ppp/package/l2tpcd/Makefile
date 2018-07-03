#
# L2TP module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# Description: ecos Makefile of L2TP module
# Author : zhuhuan
# Version: 1.0
# Created Time: Tue 19 Jan 2016 14:21:36 PM CST
#


MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

# router config
include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#
PPPDIR = $(TENDA_APP_DIR)/cbb/src/ppp/package
EXTRA_CFLAGS = -I$(shell pwd) -I$(PPPDIR)/pppd -I$(TENDA_APP_DIR)/include/crypto
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared
EXTRA_CFLAGS += -D_KERNEL -DBCM47XX -D__ECOS -DROUTER_TINY -DCFG_GMAC -DBCMWPA2
EXTRA_CFLAGS += -I$(PPPDIR)/include -I$(TENDA_APP_DIR)/net_drive/include
EXTRA_CFLAGS += -include config.h
#
# Enumerate files to compile
#
ALL_OBJS = avp.o tunnel.o session.o l2tp_ecos.o l2tp_mainloop.o l2tp_fsm.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(ENDIAN) -o $@ $(ALL_OBJS) 

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean