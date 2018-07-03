#
# ecos router ipfilter tools Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-08-20 04:09:29 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


IPF_PKG := ip_fil3.4.35

all: $(MODULE)

include $(ECOS_AP_DIR)/rules.mk
include $(ECOS_WORK_DIR)/.config
include $(TENDA_APP_DIR)/.config

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/net_drive/ipfilter/$(IPF_PKG) -I$(TENDA_APP_DIR)/net_drive/ipfilter/include -I$(TENDA_APP_DIR)/router_shared -D__FreeBSD_version=440000 -DBSD=200108 -DBCM47XX -DTENDA_KDEV_ADD 
#
# Enumerate files to compile
#

ifeq ($(CONFIG_A9),y)
CMD = natcmd.o 
else
VPATH = $(TENDA_APP_DIR)/net_drive/ipfilter/$(IPF_PKG)
#IPNAT = ipnat.o natparse.o common.o
#IPF = parse.o opt.o facpri.o
#IPNAT += printnat.o
#FILS = printstate.o
CMD = natcmd.o filcmd.o
endif

ifdef CONFIG_IPFILTER_LOG
EXTRA_CFLAGS += -DIPFILTER_LOG
endif

ALL_OBJS = $(IPNAT) $(IPF) $(CMD)
#
# Leave rule to make
#

#$(MODULE): $(ALL_OBJS:.o=.d)
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif

clean:
	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
	-rm -f $(MODULE)
	
.PHONY: clean